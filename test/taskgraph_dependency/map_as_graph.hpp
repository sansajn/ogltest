/*! \file Adaptuje std::map<X, edge_list<X>> pre ako incidence|adjacency|vertex_list graph model. */
#include <map>
#include <boost/graph/graph_traits.hpp>
#include <boost/iterator/iterator_facade.hpp>

namespace boost {

// mal : Map Adjacency List
template <typename EdgeList>
using mal_graph = std::map<typename EdgeList::value_type, EdgeList>;

namespace detail {

//! Implementuje MultiPassInputIterator pre std::map.
template <typename I>
class key_iterator
	: public boost::iterator_facade<key_iterator<I>, typename I::value_type::first_type, std::input_iterator_tag>
{
public:
	using key_type = typename I::value_type::first_type;

	key_iterator() {}
	explicit key_iterator(I iter) : _it(iter) {}

private:
	key_type const & dereference() const {return _it->first;}
	bool equal(key_iterator<I> const & rhs) const {return _it == rhs._it;}
	void increment() {++_it;}

	I _it;

	friend class boost::iterator_core_access;
};  // key_iterator

template <typename V, typename Iter>
class mal_out_edge_iterator_impl
	: public boost::iterator_facade<mal_out_edge_iterator_impl<V, Iter>, std::pair<V, V>, std::input_iterator_tag, std::pair<V, V>>
{
public:
	using edge_t = std::pair<V, V>;

	mal_out_edge_iterator_impl() {}
	explicit mal_out_edge_iterator_impl(V source, Iter it) : _source(source), _it(it) {}

private:
	edge_t dereference() const {return edge_t(_source, *_it);}
	bool equal(mal_out_edge_iterator_impl<V, Iter> const & rhs) const {return _it == rhs._it;}
	void increment() {++_it;}

	V _source;
	Iter _it;

	friend class boost::iterator_core_access;
};  // val_out_edge_iterator

template <typename EdgeList>
using mal_vertex_iterator = key_iterator<typename mal_graph<EdgeList>::const_iterator>;

template <typename EdgeList>
using mal_out_edge_iterator = mal_out_edge_iterator_impl<typename EdgeList::value_type, typename EdgeList::const_iterator>;

template <typename EdgeList>
using mal_out_edge_return = std::pair<mal_out_edge_iterator<EdgeList>, mal_out_edge_iterator<EdgeList>>;

template <typename EdgeList>
using mal_edge = std::pair<typename EdgeList::value_type, typename EdgeList::value_type>;

}  // detail

struct map_as_graph_traversal_tag
	: public incidence_graph_tag, public adjacency_graph_tag, public vertex_list_graph_tag
{};

template <typename EdgeList>
struct graph_traits<mal_graph<EdgeList>>
{
	typedef typename EdgeList::value_type vertex_descriptor;
	typedef detail::mal_edge<EdgeList> edge_descriptor;
	typedef typename EdgeList::const_iterator adjacency_iterator;
	typedef detail::mal_out_edge_iterator<EdgeList> out_edge_iterator;
	typedef void in_edge_iterator;
	typedef void edge_iterator;
	typedef detail::mal_vertex_iterator<EdgeList> vertex_iterator;
	typedef directed_tag directed_category;
	typedef allow_parallel_edge_tag edge_parallel_category;
	typedef map_as_graph_traversal_tag traversal_category;
	typedef typename mal_graph<EdgeList>::size_type vertices_size_type;
	typedef void edges_size_type;
	typedef typename EdgeList::size_type degree_size_type;
	static vertex_descriptor null_vertex() {return vertex_descriptor();}
};  // graph_traits

template <typename EdgeList>
struct edge_property_type<mal_graph<EdgeList>>
{
	typedef void type;
};

template <typename EdgeList>
struct vertex_property_type<mal_graph<EdgeList>>
{
	typedef void type;
};

template <typename EdgeList>
struct graph_property_type<mal_graph<EdgeList>>
{
	typedef void type;
};


// IncidenceGraph

template <typename EdgeList>
detail::mal_out_edge_return<EdgeList> out_edges(typename EdgeList::value_type v,
	mal_graph<EdgeList> const & g)
{
	typedef detail::mal_out_edge_iterator<EdgeList> iter_t;
	typedef detail::mal_out_edge_return<EdgeList> return_t;
	auto & edges = g.at(v);
	return return_t(iter_t(v, edges.cbegin()), iter_t(v, edges.cend()));
}

template <typename EdgeList>
typename EdgeList::size_type out_degree(typename EdgeList::value_type v,
	mal_graph<EdgeList> const & g)
{
	return g.at(v).size();
}


// AdjacencyGraph

template <typename EdgeList>
std::pair<typename EdgeList::const_iterator, typename EdgeList::const_iterator>
adjacent_vertices(typename EdgeList::value_type v, mal_graph<EdgeList> const & g)
{
	auto & edges = g.at(v);
	return std::make_pair(edges.begin(), edges.end());
}


// VertexListGraph

template <typename EdgeList>
std::pair<detail::mal_vertex_iterator<EdgeList>, detail::mal_vertex_iterator<EdgeList>>
vertices(mal_graph<EdgeList> const & g)
{
	using iter_t = detail::mal_vertex_iterator<EdgeList>;
	return std::make_pair(iter_t(g.cbegin()), iter_t(g.cend()));
}

template <typename EdgeList>
typename std::map<int, EdgeList>::size_type num_vertices(mal_graph<EdgeList> const & g)
{
	return g.size();
}


// MutableGraph

template <typename EdgeList>
std::pair<detail::mal_edge<EdgeList>, bool> add_edge(
	typename EdgeList::value_type u, typename EdgeList::value_type v, mal_graph<EdgeList> & g)
{
	g[u].push_back(v);
	return std::make_pair(detail::mal_edge<EdgeList>(u, v), true);
}

// toto neviem pre mapu implementovat (bez kluca)

//template <typename EdgeList>
//typename EdgeList::value_type add_vertex(mal_graph<EdgeList> & g)
//{
//	int key = g.size();
//	g[key] = EdgeList();
//	return key;
//}

}  // boost
