#include "android_window.hpp"
#include <algorithm>
#include <streambuf>
#include <iostream>
#include <cassert>
#include <jni.h>
#include <android/log.h>

namespace ui {

using std::find_if;
using glm::ivec2;

int touch_list_finger_cast(event_handler::action finger_action);


void touch_list::insert(touch_event const & te)
{
	assert(te.finger_action != event_handler::action::cancel && "not implemented");

	auto it = find_if(_fingers.begin(), _fingers.end(),
		[&te](finger const & f){return f.id == te.finger_id;});

	if (it == _fingers.end())  // unikatny event
	{
		finger f;
		f.position = te.position;
		f.id = te.finger_id;
		f.state = touch_list_finger_cast(te.finger_action);
		_fingers.push_back(f);
	}
	else  // uz existujuci event
	{
		it->position = te.position;

		if (te.finger_action == event_handler::action::up)  // ak je action up, zmen stav na up
			it->state = touch_list_finger_cast(te.finger_action);
		else
			it->state |= touch_list_finger_cast(te.finger_action);  // inak pridaj stav (stav je down, alebo move)
	}
}


void touch_input::update()
{
	// zmaz move a up eventy
	for (auto it = _touches.begin(); it != _touches.end(); ++it)
	{
		if (it->state & touch_list::finger::up)
			it = _touches.erase(it);
		else
			it->state &= ~touch_list::finger::move;  // clear-move-flags
	}
}

void touch_input::touch_performed(int x, int y, int finger_id, event_handler::action a)
{
	touch_event te;
	te.position = ivec2{x,y};
	te.finger_action = a;
	te.finger_id = finger_id;
	_touches.insert(te);
}


namespace android_detail {

android_layer * active_window();
ui::event_handler::action to_touch_action(int a);

android_layer * __android_window_ptr = nullptr;

}  // android_detail


android_layer::android_layer(parameters const & params)
{
	assert(!android_detail::__android_window_ptr && "only one android window is supported");
	android_detail::__android_window_ptr = this;
}

android_layer::~android_layer()
{
	assert(android_detail::__android_window_ptr && "invalid window");
	android_detail::__android_window_ptr = nullptr;
}

void android_layer::user_input::update()
{
	touch.update();
}

void android_layer::user_input::touch_performed(int x, int y, int finger_id, event_handler::action a)
{
	touch.touch_performed(x, y, finger_id, a);
}


int touch_list_finger_cast(event_handler::action finger_action)
{
	switch (finger_action)
	{
		case event_handler::action::down: return touch_list::finger::down;
		case event_handler::action::up: return touch_list::finger::up;
		case event_handler::action::move: return touch_list::finger::move;
		case event_handler::action::cancel: return touch_list::finger::cancel;
		default:
			throw std::logic_error{"unknown touch_event"};
	}
}


namespace android_detail {

//! Stream buffer implementation for android-log
class android_log_streambuf : public std::streambuf
{
public:
	android_log_streambuf(int priority);
	~android_log_streambuf();

protected:
	int_type overflow(int_type c) override;
	int sync() override;

private:
	void flush();

	int _prio;
	static int const BUFSIZE = 1024;
	char _buf[BUFSIZE];
};

android_log_streambuf::android_log_streambuf(int priority)
	: _prio{priority}
{
	setp(_buf, _buf + BUFSIZE-1);  // rezervuj jeden znak pre '\0'
}

android_log_streambuf::~android_log_streambuf()
{
	flush();
}

android_log_streambuf::int_type android_log_streambuf::overflow(int_type c)
{
	assert(0 && "stream buffer overflow");
	flush();
	return c;
}

int android_log_streambuf::sync()
{
	if (*(pptr()-1) == '\n')
		flush();
	return 0;
}

void android_log_streambuf::flush()
{
	if (pptr() == _buf)
		return;  // buffer je prazdny

	*pptr() = '\0';
	__android_log_write(_prio, "NativeLog", _buf);
	setp(_buf, _buf + BUFSIZE-1);
}


void display();
void reshape(int width, int height);
void touch(int x, int y, int finger_id, int action);

char const * LOG_TAG = "android_layer";

android_log_streambuf __android_clog_buffer{ANDROID_LOG_DEBUG};
android_log_streambuf __android_cerr_buffer{ANDROID_LOG_ERROR};
std::streambuf * __default_clog_buffer = nullptr;
std::streambuf * __default_cerr_buffer = nullptr;


void create(int width, int height)
{
	// redirects cerr and clog to android logcat
	__default_clog_buffer = std::clog.rdbuf();
	std::clog.rdbuf(&__android_clog_buffer);
	__default_cerr_buffer = std::cerr.rdbuf();
	std::cerr.rdbuf(&__android_cerr_buffer);

	::create(width, height);
}

void destroy()
{
	std::clog.rdbuf(__default_clog_buffer);
	std::cerr.rdbuf(__default_cerr_buffer);

	::destroy();
}

void display()
{
	auto w = active_window();
	assert(w && "invalid scene");
	static_cast<pool_behaviour<android_layer> *>(w)->loop_step();
}

void reshape(int width, int height)
{
	__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "reshape(%d, %d)", width, height);
	auto w = active_window();
	assert(w && "invalid scene");
	w->reshape(width, height);
}

void touch(int x, int y, int finger_id, int action)
{
	__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "touch(x:%d, y:%d, id:%d, a:%d)", x, y, finger_id, action);
	auto w = active_window();
	assert(w && "invalid scene");
	w->touch_performed(x, y, finger_id, to_touch_action(action));
}

android_layer * active_window()
{
	return __android_window_ptr;
}

event_handler::action to_touch_action(int a)
{
	switch (a)
	{
		case 0: return ui::event_handler::action::down;
		case 1: return ui::event_handler::action::up;
		case 2: return ui::event_handler::action::move;
		default:
			throw std::logic_error{"unknown touch action"};
	}
}

}  // android_detail


}  // ui


extern "C" {

JNIEXPORT void JNICALL Java_org_libgl_wrapper_NativeScene_create(JNIEnv * env, jobject thiz,
	jint width, jint height)
{
	ui::android_detail::create(width, height);
}

JNIEXPORT void JNICALL Java_org_libgl_wrapper_NativeScene_destroy(JNIEnv * env, jobject thiz)
{
	ui::android_detail::destroy();
}

JNIEXPORT void JNICALL Java_org_libgl_wrapper_NativeScene_reshape(JNIEnv * env, jobject thiz,
	jint width, jint height)
{
	ui::android_detail::reshape(width, height);
}

JNIEXPORT void JNICALL Java_org_libgl_wrapper_NativeScene_display(JNIEnv * env, jobject thiz)
{
	ui::android_detail::display();
}

JNIEXPORT void JNICALL Java_org_libgl_wrapper_NativeScene_touch(JNIEnv * env, jobject thiz,
	jint x, jint y, jint pointerId, jint action)
{
	ui::android_detail::touch(x, y, pointerId, action);
}

}  // extern "C"
