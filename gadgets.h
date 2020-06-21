#ifndef BPTREE_GADGETS
#define BPTREE_GADGETS

#include <fstream>
#include <iostream>
#include <string>
#include <cstring>
#include <cstdio>
#include <algorithm>
#include <memory>

typedef uint64_t SL;

namespace psgi {

    // iterator

    template <class _Tp>
    struct Nonconst_traits;

    template <class _Tp>
    struct Const_traits {
        typedef _Tp value_type;
        typedef const _Tp& reference;
        typedef const _Tp* pointer;
        typedef Const_traits<_Tp> ConstTraits;
        typedef Nonconst_traits<_Tp> NonConstTraits;
    };

    template <class _Tp>
    struct Nonconst_traits {
        typedef _Tp value_type;
        typedef _Tp& reference;
        typedef _Tp* pointer;
        typedef Const_traits<_Tp> ConstTraits;
        typedef Nonconst_traits<_Tp> NonConstTraits;
    };

    // construct

    template <class T1, class T2>
    inline void construct(T1* p, const T2& value) {
        new (p) T1 (value);
    }

    template <class T>
    inline void destroy(T* pointer) {
        pointer->~T();
    }

    template <class ForwardIterator>
    inline void __destroy_aux(ForwardIterator first, ForwardIterator last, std::false_type) {
        for (; first != last; ++first) destroy(&*first);
    }

    template <class ForwardIterator>
    inline void __destroy_aux(ForwardIterator first, ForwardIterator last, std::true_type) {}

    template <class ForwardIterator, class T>
    inline void __destroy(ForwardIterator first, ForwardIterator last, T*) {
        typedef typename std::is_trivially_destructible<T> trivial_destructor;
        __destroy_aux(first, last, trivial_destructor());
    }

    template <class ForwardIterator>
    inline void destroy(ForwardIterator first, ForwardIterator last) {
        typedef typename std::iterator_traits<ForwardIterator>::value_type value_type;
        __destroy(first, last, static_cast<value_type*>(0));
    }

    inline void destroy(char*, char*) {}
    inline void destroy(wchar_t*, wchar_t*) {}

    // vector

    template<class T>
    class vector {
    public:
        typedef vector<T> self;

        typedef T* iterator;
        typedef const T* const_iterator;

        typedef T value_type;
        typedef value_type* pointer;
        typedef const value_type* const_pointer;
        typedef value_type& reference;
        typedef const value_type& const_reference;
        typedef size_t size_type;
        typedef ptrdiff_t difference_type;

    protected:
        iterator start;
        iterator finish;
        iterator end_of_storage;

        iterator insert_aux(iterator pos, const T& x) {
            if (finish != end_of_storage) {
                construct(finish, *(finish-1));
                ++finish;
                T x_copy = x;
                std::copy_backward(pos, finish - 2, finish - 1);
                *pos = x_copy;
            }
            else {
                const size_type old_size = size();
                const size_type elems_before = pos - start;
                const size_type len = (old_size != 0 ? 2 * old_size : 1);
                auto new_start = (value_type*)(operator new(len * sizeof(value_type)));
                iterator new_finish = std::uninitialized_copy(start, pos, new_start);
                construct(new_finish, x);
                ++new_finish;
                new_finish = std::uninitialized_copy(pos, finish, new_finish);

                destroy(begin(), end());
                deallocate();
                start = new_start;
                finish = new_finish;
                end_of_storage = new_start + len;
                pos = start + elems_before;
            }
            return pos;
        }
        void deallocate() {
            if (start) operator delete(start);
        }
        iterator allocate_and_fill(size_type n, const T& x) {
            auto result = (value_type*)(operator new(n * sizeof(value_type)));
            std::uninitialized_fill_n(result, n, x);
            return result;
        }
        void fill_initialize(size_type n, const T& value = T()) {
            start = allocate_and_fill(n, value);
            finish = start + n;
            end_of_storage = finish;
        }
    private:
        template <class InputIterator>
        void range_initialize(InputIterator first, InputIterator last) {
            size_type n = std::distance(first, last);
            start = (value_type*)(operator new(n * sizeof(value_type)));
            end_of_storage = start + n;
            finish = std::uninitialized_copy(first, last, start);
        }
    public:
        vector() : start(0), finish(0), end_of_storage(0) {}
        vector(size_type n, const T& value) {fill_initialize(n, value);}
        vector(int n, const T& value) {fill_initialize(n, value);}
        vector(long n, const T& value) {fill_initialize(n, value);}
        template <class InputIterator>
        vector(InputIterator first, InputIterator last) {
            range_initialize(first, last);
        }
        explicit vector(size_type n) {
            fill_initialize(n);
        }
        vector(const vector& other) {
            const size_type len = other.size();
            start = (value_type*)(operator new(len * sizeof(value_type)));
            finish = std::uninitialized_copy(other.begin(), other.end(), start);
            end_of_storage = start + len;
        }
        ~vector() {
            destroy(start, finish);
            deallocate();
        }
        vector& operator=(const vector& other) {
            if (this != &other) {
                destroy(start, finish);
                deallocate();
                const size_type len = other.size();
                start = (value_type*)(operator new(len * sizeof(value_type)));
                finish = std::uninitialized_copy(other.begin(), other.end(), start);
                end_of_storage = start + len;
            }
            return *this;
        }

        iterator begin() {return start;}
        const_iterator begin() const {return const_iterator(start);}
        const_iterator cbegin() const {return const_iterator(start);}
        iterator end() {return finish;}
        const_iterator end() const {return const_iterator(finish);}
        const_iterator cend() const {return const_iterator(finish);}

        bool empty() const {return start == finish;}
        size_type size() const {return size_type(finish - start);}
        size_type capacity() const {return size_type(end_of_storage - start);}
        reference operator[](const size_type n) {
            return *(start + difference_type(n));
        }
        const_reference operator[](const size_type n) const {
            return *(start + difference_type(n));
        }
        reference at(const size_type n) {return (*this)[n];}
        const_reference& at(const size_type n) const {return (*this)[n];}

        reference front() {
            return *(begin());
        }
        const_reference front() const {
            return *(begin());
        }
        reference back() {
            return *(end() - 1);
        }
        const_reference back() const {
            return *(end() - 1);
        }

        void push_back(const T &value) {
            if (finish != end_of_storage) {
                construct(finish, value);
                ++finish;
            }
            else insert_aux(end(), value);
        }
        void pop_back() {
            --finish;
            destroy(finish);
        }
        iterator insert(iterator pos, const T& value) {
            return insert_aux(pos, value);
        }
        iterator insert(iterator pos, size_type n, const T& x) {
            if (n != 0) {
                if (size_type(end_of_storage - finish) >= n) {
                    T x_copy = x;
                    const size_type elems_after = finish - pos;
                    iterator old_finish = finish;
                    if (elems_after > n) {
                        std::uninitialized_copy(finish - n, finish, finish);
                        finish += n;
                        std::copy_backward(pos, old_finish - n, old_finish);
                        std::fill(pos, pos + n, x_copy);
                    }
                    else {
                        std::uninitialized_fill_n(finish, n - elems_after, x_copy);
                        finish += n - elems_after;
                        std::uninitialized_copy(pos, old_finish, finish);
                        finish += elems_after;
                        std::fill(pos, old_finish, x_copy);
                    }
                }
                else {
                    const size_type old_size = size();
                    const size_type elems_before = pos - start;
                    const size_type len = old_size + std::max(old_size, n);
                    iterator new_start = (value_type*)(operator new(len * sizeof(value_type)));
                    iterator new_finish = std::uninitialized_copy(start, pos, new_start);
                    new_finish = std::uninitialized_fill_n(new_finish, n, x);
                    new_finish = std::uninitialized_copy(pos, finish, new_finish);

                    destroy(start, finish);
                    deallocate();
                    start = new_start;
                    finish = new_finish;
                    end_of_storage = new_start + len;
                    pos = start + elems_before;
                }
            }
            return pos;
        }
        iterator erase(iterator pos) {
            if (pos + 1 != end())
                std::copy(pos+1, finish, pos);
            --finish;
            destroy(finish);
            return pos;
        }
        iterator erase(iterator first, iterator last) {
            iterator i = std::copy(last, finish, first);
            destroy(i, finish);
            finish = finish - (last - first);
            return first;
        }
        void clear() {erase(begin(), end());}
        void resize(size_type new_size, const T& x) {
            if (new_size < size())
                erase(begin() + new_size, end());
            else
                insert(end(), new_size - size(), x);
        }
        void resize(size_type new_size) {resize(new_size, T());}
        void swap(self& oth) {
            std::swap(this->start, oth.start);
            std::swap(this->finish, oth.finish);
            std::swap(this->end_of_storage, oth.end_of_storage);
        }
    };

    // hash settings

    static const int __psgi_num_primes = 28;
    static const unsigned long __psgi_prime_list[__psgi_num_primes] =
            {
                    53, 97, 193, 389, 769,
                    1543, 3079, 6151, 12289, 24593,
                    49157, 98317, 196613, 393241, 786433,
                    1572869, 3145739, 6291469, 12582917, 25165843,
                    50331653, 100663319, 201326611, 402653189, 805306457,
                    1610612741, 3221225473ul, 4294967291ul
            };
    inline unsigned long __psgi_next_prime(unsigned long n) {
        const unsigned long* first = __psgi_prime_list;
        const unsigned long* last = __psgi_prime_list + __psgi_num_primes;
        const unsigned long* pos = std::lower_bound(first, last, n);
        return pos == last ? *(last - 1) : *pos;
    }
}

namespace ddl {

    // file visit

    template <int inst>
    class __default_file_visitor {
    protected:
        std::fstream _M_src_file;
    public:
        void set_path(const std::string& path) {
            if (_M_src_file.is_open()) _M_src_file.close();
            _M_src_file.open(path, std::ios::in | std::ios::out | std::ios::binary);
        }
        void initialize_path(const std::string& path) {
            _M_src_file.open(path, std::ios::in);
            if (!_M_src_file.fail()) {
                _M_src_file.close();
                std::remove(path.c_str());
            }
            else _M_src_file.close();
            _M_src_file.open(path, std::ios::out);
            _M_src_file.close();
            _M_src_file.open(path, std::ios::in | std::ios::out | std::ios::binary);
            _M_src_file.seekp(0, std::ios::beg);
            SL tmp = (SL)0;
            _M_src_file.write(reinterpret_cast<char*>(&tmp), sizeof(SL));
        }
        void delink() {
            _M_src_file.close();
        }
    protected:
        SL free_list() {
            _M_src_file.seekg(0, std::ios::beg);
            SL fr_lst = 0;
            _M_src_file.read(reinterpret_cast<char*>(&fr_lst), sizeof(SL));
            return fr_lst;
        }
        void set_free_list(SL loc) {
            _M_src_file.seekp(0, std::ios::beg);
            _M_src_file.write(reinterpret_cast<char*>(&loc), sizeof(SL));
        }
    public:
        SL allocate() {
            SL fr_lst_head = free_list();
            if (fr_lst_head) {
                _M_src_file.seekg(fr_lst_head, std::ios::beg);
                SL next_fl = 0;
                _M_src_file.read(reinterpret_cast<char*>(&next_fl), sizeof(SL));
                set_free_list(next_fl);
                return fr_lst_head;
            }
            else {
                _M_src_file.seekp(0, std::ios::end);
                return (SL)(_M_src_file.tellp());
            }
        }
        void deallocate(SL loc) {
            SL fr_lst_head = free_list();
            set_free_list(loc);
            _M_src_file.seekp(loc, std::ios::beg);
            _M_src_file.write(reinterpret_cast<char*>(&fr_lst_head), sizeof(SL));
        }
    };

    template <typename _T>
    class simple_visitor : public __default_file_visitor<0> {
    public:
        template <typename _T1>
        void write(SL loc, const _T1& data) {
            _M_src_file.seekp(loc, std::ios::beg);
            SL fr_lst = 0;
            _M_src_file.write(reinterpret_cast<char*>(&fr_lst), sizeof(SL));
            _M_src_file.write(reinterpret_cast<const char*>(&data), sizeof(_T1));
            if (sizeof(_T1) < sizeof(_T)) {
                auto len = (sizeof(_T) - sizeof(_T1)) / sizeof(char);
                char* offset = new char[len];
                std::memset(offset, 0, sizeof(_T)- sizeof(_T1));
                _M_src_file.write(offset, sizeof(_T)- sizeof(_T1));
                delete [] offset;
            }
        }
        template <typename _T1>
        SL write(const _T1& data) {
            SL loc = allocate();
            write(loc, data);
            return loc;
        }
        template <typename _T2>
        void read(SL loc, _T2* ans) {
            _M_src_file.seekg(loc + sizeof(SL), std::ios::beg);
            _M_src_file.read(reinterpret_cast<char*>(ans), sizeof(_T2));
        }
        /*
        template <typename _T2>
        void read(_T2* ans) {
            _M_src_file.read(reinterpret_cast<char*>(ans), sizeof(_T2));
        }
        template <typename _T2>
        void reader_setback(_T2*) {
            _M_src_file.seekg(-(SL)sizeof(_T2), std::ios::cur);
        }
        */
        template <typename _T3>
        void head(_T3* hd) {
            _M_src_file.seekg(sizeof(SL), std::ios::beg);
            _M_src_file.read(reinterpret_cast<char*>(hd), sizeof(_T3));
        }
        template <typename _T3>
        void set_head(_T3& new_head) {
            _M_src_file.seekp(sizeof(SL), std::ios::beg);
            _M_src_file.write(reinterpret_cast<char*>(&new_head), sizeof(_T3));
        }
    };

    // cache

    template <typename _Value>
    struct __cache_node {
        typedef __cache_node* link_type;

        link_type next;
        link_type earlier;
        link_type later;
        _Value val;
        bool updated;
    };

    template <class _Value, class _Key, class _HashFcn,
            class _ExtractKey, class _EqualKey>
    class cache {
    public:
        typedef _Value value_type;
        typedef _Key key_type;
        typedef _Value& reference;
        typedef const _Value& const_reference;
        typedef _Value* pointer;
        typedef const _Value* const_pointer;
        typedef size_t size_type;
        typedef ptrdiff_t difference_type;

        typedef _HashFcn hasher;
        typedef _EqualKey key_equal;
    private:
        typedef __cache_node<_Value> node;
        node* header;

        _HashFcn _M_hash;
        _EqualKey _M_equals;
        _ExtractKey _M_get_key;

        size_type _M_num_elems;
        size_type _M_max_size;
    public:
        size_type get_threshold() {return _M_max_size;}
    public:
        psgi::vector<node*> buckets;
    protected:
        node* get_node() {
            auto ans = (node*)(operator new(sizeof(node)));
            ans->next = 0;
            ans->later = 0;
            ans->earlier = 0;
            ans->updated = false;
            return ans;
        }
        node* create_node(const value_type& x) {
            auto n = get_node();
            psgi::construct(&(n->val), x);
            return n;
        }
        void destroy_node(node* n) {
            psgi::destroy(&n->val);
            operator delete(n);
        }
    private:
        size_type next_prime(size_type n) const {return psgi::__psgi_next_prime(n);}
        void empty_initialize(size_type n) {
            header = get_node();
            header->earlier = header;
            header->later = header;
            const size_type n_buckets = next_prime(n);
            buckets = psgi::vector<node*>(n_buckets, (node*)0);
        }
    protected:
        size_type bkt_num_key(const key_type& key, size_t n) const {
            return _M_hash(key) % n;
        }
        size_type bkt_num_key(const key_type& key) const {
            return bkt_num_key(key, buckets.size());
        }
        size_type bkt_num(const value_type& val) const {
            return bkt_num_key(_M_get_key(val));
        }
        size_type bkt_num(const value_type& val, size_t n) const {
            return bkt_num_key(_M_get_key(val), n);
        }

        std::pair<node*, bool> __insert_aux(const value_type& x) {
            const size_type n = bkt_num(x);
            node* first = buckets[n];
            for (node* cur = first; cur; cur = cur->next) {
                if (_M_equals(_M_get_key(cur->val), _M_get_key(x)))
                    return std::pair<node*, bool>(cur, false);
            }
            node* tmp = create_node(x);
            tmp->next = first;
            link_back(tmp);
            buckets[n] = tmp;

            ++_M_num_elems;
            return std::pair<node*, bool>(tmp, true);
        }
        void __rehash(size_type new_size) {
            const size_type old_size = buckets.size();
            psgi::vector<node*> tmp(new_size, (node*)0);
            for (size_type b = 0; b < old_size; ++b) {
                node* first = buckets[b];
                while(first) {
                    size_type new_bucket = bkt_num(first->val, new_size);
                    buckets[b] = first->next;
                    first->next = tmp[new_bucket];
                    tmp[new_bucket] = first;
                    first = buckets[b];
                }
            }
            buckets.swap(tmp);
        }

        void delink_node(node* p) {
            p->earlier->later = p->later;
            p->later->earlier = p->earlier;
        }
        void link_back(node* tmp) {
            header->earlier->later = tmp;
            tmp->earlier = header->earlier;
            tmp->later = header;
            header->earlier = tmp;
        }
    public:
        cache(size_type n, size_type sz, const _HashFcn& hf, const _EqualKey& eql)
                : _M_num_elems(0), _M_max_size(sz), _M_hash(hf),
                _M_equals(eql), _M_get_key(_ExtractKey()) {
            empty_initialize(n);
        }
        cache(size_type n, size_type sz, const _HashFcn& hf,
              const _EqualKey& eql, const _ExtractKey& ext)
                : _M_num_elems(0), _M_max_size(sz), _M_hash(hf),
                _M_equals(eql), _M_get_key(ext) {
            empty_initialize(n);
        }
        ~cache() {
            clear();
            operator delete(header);
        }

        node* begin() {return header->later;}
        node* end() {return header;}

        size_type bucket_count() const {return buckets.size();}
        size_type size() const {return _M_num_elems;}
        bool empty() const {return _M_num_elems==0;}
        hasher hash_func() const {return _M_hash;}
        key_equal key_eq() const {return _M_equals;}

        float load_factor() const {return (float)size()/(float)bucket_count();}
        float max_load_factor() const {return 1.0f;}

        /*
        void need_updation(const key_type& key) {
            node* tmp = find(key);
            if (tmp != end()) {
                tmp->updated = true;
            }
        }
         */

        node* find(const key_type& key) {
            size_type n = bkt_num_key(key);
            node* first = buckets[n];
            for (; first && !_M_equals(_M_get_key(first->val), key); first = first->next);
            if (first) {
                delink_node(first);
                link_back(first);
                return first;
            }
            else {return end();}
        }
        std::pair<node*, bool> insert(const value_type& val) {
            resize(_M_num_elems + 1);
            return __insert_aux(val);
        }
        size_type erase(const key_type& key) {
            const size_type n = bkt_num_key(key);
            node* first = buckets[n];

            size_type erased = 0;
            node* cur = first;
            for (; cur && cur->next;) {
                if (_M_equals(_M_get_key(cur->next->val), key)) {
                    node* tmp = cur->next;
                    cur->next = tmp->next;
                    delink_node(tmp);
                    destroy_node(tmp);
                    ++erased;
                }
                else cur = cur->next;
            }
            if (_M_equals(_M_get_key(first->val), key)) {
                buckets[n] = first->next;
                delink_node(first);
                destroy_node(first);
                ++erased;
            }

            _M_num_elems -= erased;
            return erased;
        }
        void erase(node* p) {
            if (p != end()) {
                size_type n = bkt_num(p->val);
                node* cur = buckets[n];
                if (cur == p) {
                    buckets[n] = p->next;
                }
                else {
                    while (cur->next != p) cur = cur->next;
                    cur->next = p->next;
                }
                delink_node(p);
                destroy_node(p);
                --_M_num_elems;
            }
        }

        void resize(size_type num_elements) {
            if ( ((float)num_elements/(float)bucket_count()<=max_load_factor()) &&
                 (load_factor()<=max_load_factor()) ) {return;}
            auto new_buckets
                    = (size_type)((float)(std::max(size(), num_elements)) / max_load_factor());
            const size_type new_size = next_prime(new_buckets);
            __rehash(new_size);
        }
        void clear() {
            for (size_type i = 0, it = buckets.size(); i < it; ++i) {
                node* cur = buckets[i];
                while (cur != 0) {
                    node* next = cur->next;
                    destroy_node(cur);
                    cur = next;
                }
                buckets[i] = 0;
            }
            _M_num_elems = 0;
        }
    };
}

#endif