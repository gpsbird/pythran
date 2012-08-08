#ifndef PYTHONIC_INTRINSICS_H
#define PYTHONIC_INTRINSICS_H

#include <tuple>
#include <sstream>
#include <complex>

namespace pythonic {

    /* abs */
    using std::abs;
    PROXY(pythonic, abs);

    /* all */
    template <class T>
        bool all( core::list<T> const& s) {
            return s.end() == std::find_if(s.begin(), s.end(), [](T const&t) { return not bool(t); });
        }
    PROXY(pythonic, all);

    /* any */
    template <class T>
        bool any( core::list<T> const& s) {
            return s.end() != std::find_if(s.begin(), s.end(), [](T const&t) { return bool(t); });
        }
    PROXY(pythonic, any);

    /* bin */
    template<class T>
        std::string bin(T const &v) {
            size_t i = 1 << (8*sizeof(T)-1);
            while(i and not (v&i)) i>>=1;
            if(not i) return "0b0";
            else {
                std::ostringstream oss;
                oss << "0b";
                for(; i ; i>>=1)
                    if(v & i) oss << "1";
                    else oss << "0";
                return oss.str();
            }
        }
    PROXY(pythonic, bin);

    /* chr */
    template<class T>
        std::string chr(T const &v) { return std::string(1,(char)v); }
    PROXY(pythonic, chr);

    /* cmp */
    template<class T0, class T1>
        long cmp(T0 const& v0, T1 const& v1) {
            return v0 == v1 ? 0 : ( v0 < v1 ? -1 : 1 );
        }
    PROXY(pythonic, cmp);

    /* complex */
    template<class T0, class T1=double> // do not handle string as first argument
    std::complex<double> complex(T0 const& v0, T1 const& v1 = 0.) {
        return std::complex<double>(v0,v1);
    }
    PROXY(pythonic, complex);

    /* divmod */
    template<class T0, class T1>
        auto divmod(T0 const& t0, T1 const& t1) // other types are left over
            -> decltype(std::make_tuple(t0/t1, t0%t1)) {
                return std::make_tuple(t0/t1, t0%t1);
            }
    PROXY(pythonic, divmod);

    /* enumerate */
    template<class T>
    struct enumerate_iterator : std::iterator< std::random_access_iterator_tag, long >{
        long value;
        typename core::list<T>::const_iterator iter;
        enumerate_iterator(){}
        enumerate_iterator(long value, typename core::list<T>::const_iterator iter) : value(value), iter(iter) {}
        std::tuple<long,T>  operator*() { return std::make_tuple(value, *iter); }
        enumerate_iterator& operator++() { ++value,++iter; return *this; }
        enumerate_iterator operator++(int) { enumerate_iterator self(*this); ++value, ++iter; return self; }
        enumerate_iterator& operator+=(long n) { value+=n,iter+=n; return *this; }
        bool operator!=(enumerate_iterator const& other) { return value != other.value; }
        bool operator<(enumerate_iterator const& other) { return value < other.value; }
        long operator-(enumerate_iterator const& other) { return value - other.value; }
    };

    template <class T>
    struct _enumerate {
        core::list<T> seq;
        _enumerate() {}
        typedef std::tuple<long, T> value_type;
        _enumerate( core::list<T> const& seq ) : seq(seq) {}
        enumerate_iterator<T> begin() const { return enumerate_iterator<T>(0L,seq.begin()); }
        enumerate_iterator<T> end() const { return enumerate_iterator<T>(seq.end()-seq.begin(), seq.end()); }
    };
    template <class T>
    _enumerate<T> enumerate(core::list<T> const& seq) { return _enumerate<T>(seq); }
    PROXY(pythonic,enumerate);

    /* filter */
    template<class F, class Iterable>
        core::list<typename Iterable::iterator::value_type> filter(F const& f, Iterable const& iterable) { /* does not implement the full standard */
            core::list<typename Iterable::iterator::value_type> out;
            std::copy_if(iterable.begin(), iterable.end(), std::back_inserter(out), f);
            return out;
        }

    PROXY(pythonic, filter);

    /* hex */
    template <class T>
        std::string hex(T const & v) {
            std::ostringstream oss;
            oss << "0x" << std::hex << v;
            return oss.str();
        }
    PROXY(pythonic, hex);

    /* id */
    template <class T>
        struct _id {
            intptr_t operator()(T const &t) {
                return reinterpret_cast<intptr_t>(&t);
            }
        };
    template <class T>
        struct _id< core::list<T> > {
            intptr_t operator()(core::list<T> const &t) {
                return reinterpret_cast<intptr_t>(t.data);
            }
        };

    template <class T>
        intptr_t id(T const & t) {
            return _id<T>()(t);
        }
    PROXY(pythonic,id);

    /* in */
    template <class T, class V>
        bool in(T const &t, V const &v) {
            return std::find(t.begin(), t.end(), v) != t.end();
        }
    template <>
        bool in<std::string, std::string>(std::string const &t, std::string const &v) {
            return t.find(v) != std::string::npos;
        }
    PROXY(pythonic,in);


    /* len */

    template <class T, class category>
        struct _len {
        };
    template <class T>
        struct _len<T,std::random_access_iterator_tag> {
            long operator()(T const &t) {
                return t.end() - t.begin();
            }
        };
    template <class T>
        struct _len<T,std::forward_iterator_tag> {
            long operator()(T const &t) {
#ifndef NDEBUG
                std::cout << "bad:" << __PRETTY_FUNCTION__ << std::endl;
#endif
                return std::distance(t.begin(), t.end());
            }
        };

    template <class T>
        struct _len<core::set<T>, std::bidirectional_iterator_tag> {
            long operator()(core::set<T> const &t) {
                return t.size();
            }
        };
    template <class T>
        long len(T const &t) {
            return _len<T, typename std::iterator_traits<decltype(const_cast<T*>(&t)->begin())>::iterator_category >()(t);
        }
    PROXY(pythonic,len);

    /* list */
    core::empty_list list() {
        return core::empty_list();
    }

    template <class Iterable>
        core ::list<typename Iterable::iterator::value_type> list(Iterable const & t) {
            return core::list<typename Iterable::iterator::value_type>(t.begin(), t.end());
        } 
    PROXY(pythonic,list);

    /* tuple */
    template <class Iterable>
        struct _tuple {
            core::list<typename Iterable::value_type> operator()(Iterable i) {
                return core::list<typename Iterable::value_type>(i.begin(), i.end());
            }
        };

    template <class T>
        struct _tuple<core::list<T>> {
            core::list<T> operator()(core::list<T> i) {
                return core::list<T>(i.begin(), i.end());
            }
        };
    template <class Iterable> /* this is far from perfect, but how to cope with the difference between python tuples and c++ ones ? */
        core::list<typename Iterable::iterator::value_type> tuple(Iterable i) {
            return _tuple<Iterable>()(i);
        }
    PROXY(pythonic, tuple);

    /* set */
    core::empty_set set() {
        return core::empty_set();
    }

    template <class Iterable>
        struct _set {
            typedef core::set<typename Iterable::value_type> type;
            type operator()(Iterable &t) {
                return type(t.begin(), t.end());
            }
        };

    template <class Iterable>
        typename _set<Iterable>::type set(Iterable t) {
            return _set<Iterable>()(t);
        } 
    PROXY(pythonic,set);



    /* map */
    template <typename Operator, typename List0, typename... Iterators>
        auto _map(Operator& op, List0 const& seq, Iterators... iterators)
        -> core::list< decltype(op(*seq.begin(), *iterators...)) >
        {
            decltype(_map(op,seq, iterators...)) s(len(seq));
            auto iter = s.begin();
            for(auto & iseq : seq)
                *iter++= op(iseq, *iterators++...);
            return s;
        }

    template <typename Operator, typename List0, typename... ListN>
        auto map(Operator op, List0 const& seq, ListN const &... lists)
        -> decltype( _map(op, seq, lists.begin()...) )
        {
            return _map(op, seq, lists.begin()...);
        }

    PROXY(pythonic,map);

    /* max */
    template <class... Types> struct Max;

    template <class T0>
        struct Max<T0> {
            typedef T0 return_type;
            return_type operator()(T0 const & t) {
                return t;
            }
        };
    template <class T0, class T1>
        struct Max<T0, T1> {
            typedef decltype(std::declval<T0>() + std::declval<T1>()) return_type;
            return_type operator()(T0 const & t0, T1 const& t1) {
                return t0>t1? t0: t1;
            }

        };

    template <class T0, class... Types>
        struct Max<T0, Types...> {
            typedef decltype( Max<T0, typename Max<Types...>::return_type >::return_type) return_type;


            return_type operator()(T0 const & t0, Types const&... values)
            {
                auto t1 = (*this)(values...);
                return t0 > t1 ? t0 : t1;
            }
        };
    template<class T>
        struct Max<core::list<T>> {
            typedef typename core::list<T>::value_type return_type;

            return_type operator()( core::list<T> const & s ) {
                return *std::max_element(s.begin(), s.end());
            }
        };

    template <class... Types>
        typename Max<Types...>::return_type max(Types const&... values) {
            return Max<Types...>()(values...);
        }
    PROXY(pythonic,max);

    /* min */
    template <class... Types> struct Min;

    template <class T0>
        struct Min<T0> {
            typedef T0 return_type;
            return_type operator()(T0 const & t) {
                return t;
            }
        };
    template <class T0, class T1>
        struct Min<T0, T1> {
            typedef decltype(std::declval<T0>() + std::declval<T1>()) return_type;
            return_type operator()(T0 const & t0, T1 const& t1) {
                return t0>t1? t1: t0;
            }

        };

    template <class T0, class... Types>
        struct Min<T0, Types...> {
            typedef decltype( Min<T0, typename Min<Types...>::return_type >::return_type) return_type;


            return_type operator()(T0 const & t0, Types const&... values)
            {
                auto t1 = (*this)(values...);
                return t0 > t1 ? t0 : t1;
            }
        };

    template<class T>
        struct Min<core::list<T>> {
            typedef typename core::list<T>::value_type return_type;

            return_type operator()( core::list<T> const & s ) {
                return *std::min_element(s.begin(), s.end());
            }
        };

    template <class... Types>
        typename Min<Types...>::return_type min(Types const&... values) {
            return Min<Types...>()(values...);
        }
    PROXY(pythonic,min);

    /* oct */
    template <class T>
        std::string oct(T const & v) {
            std::ostringstream oss;
            oss << '0' << std::oct << v;
            return oss.str();
        }
    PROXY(pythonic, oct);

    /* pow */
    using std::pow;
    PROXY(pythonic, pow);

    /* xrange */
    struct xrange_iterator : std::iterator< std::random_access_iterator_tag, long >{
        long value;
        long step;
        xrange_iterator() {}
        xrange_iterator(long v, long s) : value(v), step(s) {}
        long& operator*() { return value; }
        xrange_iterator& operator++() { value+=step; return *this; }
        xrange_iterator operator++(int) { xrange_iterator self(*this); value+=step; return self; }
        xrange_iterator& operator+=(long n) { value+=step*n; return *this; }
        bool operator!=(xrange_iterator const& other) { return value != other.value; }
        bool operator<(xrange_iterator const& other) { return value < other.value; }
        long operator-(xrange_iterator const& other) { return (value - other.value)/step; }
    };

    struct xrange {
        long _begin;
        long _end;
        long _step;
        typedef long value_type;
        typedef xrange_iterator iterator;
        typedef xrange_iterator const_iterator;
        typedef xrange_iterator reverse_iterator;
        typedef xrange_iterator const_reverse_iterator;
        xrange(){}
        xrange( long b, long e , long s=1) : _begin(b), _end(e), _step(s) {}
        xrange( long e ) : _begin(0), _end(e), _step(1) {}
        xrange_iterator begin() const { return xrange_iterator(_begin, _step); }
        xrange_iterator end() const {
            if(_step>0) return xrange_iterator(_begin + std::max(0L,_step * ( (_end - _begin + _step -1)/ _step)) , _step);
            else return xrange_iterator(_begin + std::min(0L,_step * ( (_end - _begin + _step +1)/ _step)) , _step);
        }
        xrange_iterator rbegin() const { return xrange_iterator(_end-_step, -_step); }
        xrange_iterator rend() const {
            if(_step>0) return xrange_iterator(_end - std::max(0L,_step * ( 1 + (_end - _begin)/ _step)) , -_step);
            else return xrange_iterator(_end - std::min(0L,_step * ( 1 + (_end - _begin)/ _step)) , -_step);
        }
    };
    PROXY(pythonic,xrange);

    /* range */
    core::list<long> _range(xrange & xr) {
        core::list<long> s(len(xr));
        std::copy(xr.begin(), xr.end(), s.begin());
        return s;
    }

    core::list<long> range(long e) {
        xrange xr(e);
        return _range(xr);
    }
    core::list<long> range(long b, long e, long s=1) {
        xrange xr(b, e, s);
        return _range(xr);
    }
    PROXY(pythonic,range);

    /* reduce */

    template<class Iterable, class Operator>
        auto reduce(Operator op, Iterable s)
        -> decltype( op( std::declval< typename Iterable::iterator::value_type >(), std::declval< typename Iterable::iterator::value_type >() ) )
        {
            auto iter = s.begin();
            auto r = *iter;
            ++iter;
            if(iter!=s.end())
                return  std::accumulate(iter, s.end(), r, op);
            else
                return r;
        }
    template<class Iterable, class Operator, class T>
        auto reduce(Operator op, Iterable s, T const & init)
        -> decltype( std::accumulate(s.begin(), s.end(), static_cast<decltype(op(init,std::declval<typename Iterable::iterator::value_type>()))>(init), op) )
        {
            return std::accumulate(s.begin(), s.end(), static_cast<decltype(op(init,std::declval<typename Iterable::iterator::value_type>()))>(init), op);
        }
    PROXY(pythonic,reduce);

    /* reversed */
    template <class Iterable>
        struct _reversed {
            Iterable iterable;
            _reversed() {}
            _reversed(Iterable const& iterable) : iterable(iterable) {}
            typedef typename Iterable::value_type value_type;
            typedef typename Iterable::reverse_iterator iterator;
            typedef typename Iterable::const_reverse_iterator const_iterator;
            iterator begin() { return iterable.rbegin(); }
            iterator end() { return iterable.rend(); }
            const_iterator begin() const { return iterable.rbegin(); }
            const_iterator end() const { return iterable.rend(); }
        };


    template <class Iterable>
        _reversed<Iterable> reversed(Iterable const& iterable) {
            return _reversed<Iterable>(iterable);
        }
    PROXY(pythonic, reversed);


    /* round */
    template <class T>
        double round(T const & v, size_t n=0) {
            T p = std::pow(10,n);
            return std::lround(v * p)/p;
        }
    PROXY(pythonic, round);

    /* sorted */
    template <class T>
        core::list<T> sorted(core::list<T> const& seq) {
            core::list<T> out(seq);
            std::sort(out.begin(), out.end());
            return out;
        }
    template <class T, class C>
        core::list<T> sorted(core::list<T> const& seq, C const& cmp) {
            core::list<T> out(seq);
            std::sort(seq.bein(), seq.end(), cmp);
            return out;
        }
    PROXY(pythonic, sorted);

    /* str */
    template <class T>
        std::string str(T const & t) {
            std::ostringstream oss;
            oss << t;
            return oss.str();
        }
    PROXY(pythonic, str);

    /* sum */
    template<class Iterable, class T>
        auto sum(Iterable s, T start) -> decltype(start+std::declval<typename Iterable::iterator::value_type>())
        {
            return std::accumulate(s.begin(), s.end(), (decltype(start+*s.begin()))start);
        }
    template<class Iterable>
        typename Iterable::iterator::value_type sum(Iterable s)
        {
            return sum(s,0L);
        }
    PROXY(pythonic,sum);

    /* xrange -> forward declared */

    /* zip */
    template<class Iterator0, class... Iterators>
        core::list< std::tuple<typename Iterator0::value_type, typename Iterators::value_type... > > _zip(Iterator0 first, Iterator0 last, Iterators...  iters) {
            core::list< std::tuple< typename Iterator0::value_type, typename Iterators::value_type... > > out(last-first);
            auto iter = out.begin();
            while(first!=last)
                *iter++= std::make_tuple( *first++, *iters++... );
            return out;
        }

    template<class List0, class... Lists>
        core::list< std::tuple<typename List0::value_type, typename Lists::value_type... > > zip(List0 const& s0, Lists const&...  lists) {
            return _zip(s0.begin(), s0.end(), lists.begin()...);
        }

    core::empty_list zip() {
        return core::empty_list();
    }

    PROXY(pythonic,zip);

    /* reserve */
    template <class Container, class From>
        void reserve(Container & , From &) //do nothing unless specialized
        {
        }
    template <class T, class From>
        void reserve(core::list<T> & l, From &f, typename From::const_iterator p=typename From::const_iterator())
        {
            l.reserve(len(f));
        }

}
#endif
