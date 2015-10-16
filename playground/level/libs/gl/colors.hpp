#pragma once

// color codes
#include <glm/vec3.hpp>

namespace rgb {

// basic colors
glm::vec3 const black{0, 0, 0};
glm::vec3 const white{1, 1, 1};
glm::vec3 const red{1, 0, 0};
glm::vec3 const lime{0, 1, 0};
glm::vec3 const blue{0, 0, 1};
glm::vec3 const yellow{1, 1, 0};
glm::vec3 const cyan{0, 1, 1};
glm::vec3 const magenta{1, 0, 1};
glm::vec3 const silver{0.7529, 0.7529, 0.7529};
glm::vec3 const gray{0.5020, 0.5020, 0.5020};
glm::vec3 const maroon{0.5020, 0, 0};
glm::vec3 const olive{0.5020, 0.5020, 0};
glm::vec3 const green{0, 0.5020, 0};
glm::vec3 const purple{0.5020, 0, 0.5020};
glm::vec3 const teal{0, 0.5020, 0.5020};
glm::vec3 const navy{0 , 0, 0.5020};

namespace yellow_shades {

glm::vec3 const yellow{1, 1, 0};
glm::vec3 const light_yellow{1, 1, 0.8784};
glm::vec3 const lemon_chiffon{1, 0.9804, 0.8039};
glm::vec3 const light_goldenrod_yellow{0.9804, 0.9804, 0.8235};
glm::vec3 const papaya_whip{1, 0.9373, 0.8353};
glm::vec3 const moccasin{1, 0.8941, 0.7098};
glm::vec3 const peach_puff{1, 0.8549, 0.7255};
glm::vec3 const pale_goldenrod{0.9333, 0.9098, 0.6667};
glm::vec3 const khaki{0.9412, 0.9020, 0.5490};
glm::vec3 const dark_khaki{0.7412, 0.7176, 0.4196};
glm::vec3 const olive{0.5020, 0.5020, 0};
glm::vec3 const green_yellow{0.6784, 1, 0.1843};
glm::vec3 const yellow_green{0.6039, 0.8039, 0.1961};

} // yellow_shades

}  // rgb
