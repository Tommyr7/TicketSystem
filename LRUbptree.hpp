//b+tree in ssd

#ifndef LRU_BPLUS_TREE
#define LRU_BPLUS_TREE

#include <fstream>
#include <string>
#include <utility>
#include <algorithm>
#include <functional>
#include <iostream>
#include <cstddef>
#include <memory>

#ifndef BPTREE_GADGETS
#   include "gadgets.h"
#endif

#define BLOCK_SIZE 4096
#define CACHE_INITIAL_SIZE_BLOCK 20
#define CACHE_MAX_SIZE_BLOCK 35
#define CACHE_INNITIAL_SIZE_DATA 20
#define CACHE_MAX_SIZE_DATA 35

#define MAX(a, b) ((a) < (b) ? (b) : (a))
enum NODE_TYPE {LEAF, INNER};

template <typename Pair>
struct select1st {
    typename Pair::first_type operator() (const Pair &x) const {
        return x.first;
    }
};

template <typename _Key>
struct __LRUBPTREE_DEFAULT_TRAITS {
    static const int slots_max = MAX(8, BLOCK_SIZE / (sizeof(_Key) + sizeof(SL)));
    static const int slots_min = slots_max / 2;
};

struct __LRUBPTREE_NODE {
    SL pos;
    NODE_TYPE _M_type;
    size_t key_num;

    __LRUBPTREE_NODE() : _M_type(LEAF), key_num(0), pos(0) {}
    ~__LRUBPTREE_NODE() {pos = 0; key_num = 0;}
    __LRUBPTREE_NODE(const __LRUBPTREE_NODE& oth)
        : key_num(oth.key_num), _M_type(oth._M_type), pos(oth.pos) {}
    __LRUBPTREE_NODE& operator=(const __LRUBPTREE_NODE& oth) {
        if (this != &oth) {
            _M_type = oth._M_type;
            key_num = oth.key_num;
            pos = oth.pos;
        }
        return *this;
    }
};

template <typename _Key, typename _Traits>
struct __LRUBPTREE_INNER_NODE : public __LRUBPTREE_NODE {
    typedef _Key key_type;
    typedef _Traits traits;
    typedef SL loc_ptr;
    typedef __LRUBPTREE_INNER_NODE<_Key, _Traits> self;

    static const int maxinnerslots = traits::slots_max;
    static const int mininnerslots = traits::slots_min;

    key_type keys[maxinnerslots - 1];
    loc_ptr child_id[maxinnerslots];

    __LRUBPTREE_INNER_NODE() : __LRUBPTREE_NODE() {
        _M_type = INNER;
        for (int i = 0; i < maxinnerslots; ++i) {
            child_id[i] = 0;
        }
    }
    ~__LRUBPTREE_INNER_NODE() {
        psgi::destroy(keys, keys + key_num);
        pos = 0;
        key_num = 0;
    }
    __LRUBPTREE_INNER_NODE(const __LRUBPTREE_INNER_NODE& oth) {
        pos = oth.pos;
        key_num = oth.key_num;
        _M_type = INNER;
        std::uninitialized_copy(oth.keys,
                                oth.keys + key_num, keys);
        std::copy(oth.child_id, oth.child_id + key_num + 1, child_id);
    }
    __LRUBPTREE_INNER_NODE& operator=(const __LRUBPTREE_INNER_NODE& oth) {
        if (this != &oth) {
            ~__LRUBPTREE_INNER_NODE();
            pos = oth.pos;
            key_num = oth.key_num;
            _M_type = INNER;
            std::uninitialized_copy(oth.keys,
                                    oth.keys + key_num, keys);
            std::copy(oth.child_id, oth.child_id + key_num + 1, child_id);
        }
        return *this;
    }

    inline bool is_full() const {return key_num == maxinnerslots - 1;}
    inline bool is_few() const {return key_num < mininnerslots;}

    bool operator==(const self& x) const {return this->pos==x.pos;}
    bool operator!=(const self& x) const {return this->pos!=x.pos;}
};

template <typename _Key, typename _Traits>
struct __LRUBPTREE_LEAF_NODE : public __LRUBPTREE_NODE {
    typedef _Key key_type;
    typedef _Traits traits;
    typedef SL loc_ptr;
    typedef __LRUBPTREE_LEAF_NODE<_Key, _Traits> self;

    static const int maxleafslots = traits::slots_max;
    static const int minleafslots = traits::slots_min;

    loc_ptr prev;
    loc_ptr next;
    key_type keys[maxleafslots];
    loc_ptr data[maxleafslots];

    __LRUBPTREE_LEAF_NODE() : __LRUBPTREE_NODE() {
        prev = next = 0;
        for (int i = 0; i < maxleafslots; ++i) {
            data[i] = 0;
        }
    }
    ~__LRUBPTREE_LEAF_NODE() {
        psgi::destroy(keys, keys + key_num);
        pos = 0;
        key_num = 0;
    }
    __LRUBPTREE_LEAF_NODE(const __LRUBPTREE_LEAF_NODE& oth)
        : prev(oth.prev), next(oth.next) {
        pos = oth.pos;
        key_num = oth.key_num;
        _M_type = LEAF;
        std::uninitialized_copy(oth.keys,
                                oth.keys + key_num, keys);
        std::copy(oth.data, oth.data + key_num, data);
    }
    __LRUBPTREE_LEAF_NODE& operator=(const __LRUBPTREE_LEAF_NODE& oth) {
        if (&oth != this) {
            ~__LRUBPTREE_LEAF_NODE();
            prev = oth.prev;
            next = oth.next;
            pos = oth.pos;
            key_num = oth.key_num;
            _M_type = LEAF;
            std::uninitialized_copy(oth.keys,
                                    oth.keys + key_num, keys);
            std::copy(oth.data, oth.data + key_num, data);
        }
        return *this;
    }

    inline bool is_full() const {return key_num == maxleafslots;}
    inline bool is_few() const {return key_num <= minleafslots;}

    bool operator==(const self& x) const {return this->pos==x.pos;}
    bool operator!=(const self& x) const {return this->pos!=x.pos;}
};

template <typename _Key, typename _Data,
        typename _Compare = std::less<_Key>,
        typename _Traits = __LRUBPTREE_DEFAULT_TRAITS<_Key>>
class LRUBPTree;

template <typename _Key, typename _Data, typename _Compare, typename _Traits>
struct __LRUBPTREE_ITERATOR {
    typedef __LRUBPTREE_ITERATOR<_Key, _Data, _Compare, _Traits> iterator;

    typedef _Key key_type;
    typedef _Data data_type;
    typedef std::pair<_Key, _Data> value_type;

    typedef value_type* pointer;
    typedef value_type& reference;
    typedef ptrdiff_t difference_type;
    typedef std::bidirectional_iterator_tag iterator_category;

    typedef __LRUBPTREE_LEAF_NODE<_Key, _Traits> leaf_type;
    leaf_type* curleaf;
    SL curpos;
    int curslot;
    mutable value_type tempvalue;

    typedef LRUBPTree<_Key, _Data, _Compare, _Traits> tree_type;
    tree_type* tree;

    __LRUBPTREE_ITERATOR()
        : curslot(0), tree(0), curleaf(), curpos(0),
          tempvalue(value_type(key_type(), data_type())) {}
    __LRUBPTREE_ITERATOR(leaf_type* l, int s, tree_type* t)
        : curslot(s), tree(t), curleaf(l),
          tempvalue(value_type(key_type(), data_type())) {
        if (curleaf) curpos = l->pos;
        else curpos = 0;
    }
    __LRUBPTREE_ITERATOR(const iterator& oth)
        : curleaf(oth.curleaf), curslot(oth.curslot), curpos(oth.curpos),
          tree(oth.tree), tempvalue(oth.tempvalue) {}
    __LRUBPTREE_ITERATOR& operator=(const __LRUBPTREE_ITERATOR& oth) {
        if (&oth != this) {
            this->curleaf = oth.curleaf;
            this->curpos = oth.curpos;
            this->curslot = oth.curslot;
            this->tree = oth.tree;
        }
        return *this;
    }

    const key_type& key() const {
        return curleaf->keys[curslot];
    }
    data_type& data(bool adjust = false) {
        curleaf = tree->request_leaf(curpos);
        return *(tree->request_data(curleaf->data[curslot], adjust));
    }
    reference operator*() {
        tempvalue.first = key();
        tempvalue.second = data();
        return tempvalue;
    }
    pointer operator->() {return &(operator*());}

    iterator& operator++() {
        if (curslot + 1 < curleaf->key_num) {
            ++curslot;
        }
        else if (curleaf->next != 0) {
            curleaf = tree->request_leaf(curleaf->next);
            curpos = curleaf->pos;
            curslot = 0;
        }
        else {
            curslot = curleaf->key_num;
        }
        return *this;
    }
    iterator operator++(int) {
        iterator tmp = *this;
        ++*this;
        return tmp;
    }
    iterator& operator--() {
        if (curslot > 0) {
            curleaf = tree->request_leaf(curpos);
            --curslot;
        }
        else if (curleaf->prev != 0) {
            curleaf = tree->request_leaf(curleaf->prev);
            curpos = curleaf->pos;
            curslot = curleaf->key_num - 1;
        }
        else {
            curleaf = tree->request_leaf(curpos);
            curslot = 0;
        }
        return *this;
    }
    iterator operator--(int) {
        iterator tmp = *this;
        --*this;
        return tmp;
    }

    bool operator==(const iterator& oth) {
        if (tree!=oth.tree || curslot!=oth.curslot) return false;
        //if (curleaf && oth.curleaf) return curleaf->pos == oth.curleaf->pos;
        return curpos == oth.curpos;
    }
    bool operator!=(const iterator& oth) {return !(*this==oth);}
};

template <typename _Key, typename _Data,
        typename _Compare, typename _Traits>
class LRUBPTree {
public:
    typedef _Key key_type;
    typedef _Data data_type;
    typedef _Compare key_compare;

    typedef size_t size_type;
    typedef std::pair<_Key, _Data> value_type;

    typedef SL loc_ptr;
    typedef __LRUBPTREE_NODE node;
    typedef node* base_ptr;
    typedef __LRUBPTREE_LEAF_NODE<_Key, _Traits> leaf_node;
    typedef leaf_node* leaf_ptr;
    typedef __LRUBPTREE_INNER_NODE<_Key, _Traits> inner_node;
    typedef inner_node* inner_ptr;

    typedef __LRUBPTREE_ITERATOR<_Key, _Data, _Compare, _Traits> iterator;
private:
    struct HEADER {
        loc_ptr _M_root;
        loc_ptr _M_head_leaf;
        loc_ptr _M_tail_leaf;
        key_compare _M_key_cmp;
        size_type _M_item_count;
        NODE_TYPE _M_root_type;

        HEADER() : _M_root(0), _M_head_leaf(0), _M_tail_leaf(0),
            _M_key_cmp(_Compare()), _M_item_count(0),
            _M_root_type(LEAF) {}
        explicit HEADER(const key_compare& cmp)
            : _M_root(0), _M_head_leaf(0), _M_tail_leaf(0),
            _M_key_cmp(cmp), _M_item_count(0),
            _M_root_type(LEAF) {}
    };
    HEADER header;
    base_ptr root() {
        if (header._M_root==0) return nullptr;
        if (header._M_root_type==LEAF)
            return request_leaf(header._M_root);
        else return request_inner(header._M_root);
    }
public:
    inline key_compare key_comp() const {return header._M_key_cmp;}
    inline bool key_equal(const key_type& a, const key_type& b) const {
        return (!key_comp()(a, b) && !key_comp()(b, a));
    }
protected:
    typedef ddl::simple_visitor<leaf_node> NODE_VIS;
    NODE_VIS node_visitor;
    typedef ddl::simple_visitor<data_type> DATA_VIS;
    DATA_VIS data_visitor;

    struct EXTRACT_POS_LEAF {
        SL operator()(const leaf_node& x) const {return x.pos;}
    };
    struct EXTRACT_POS_INNER {
        SL operator()(const inner_node& x) const {return x.pos;}
    };

    typedef ddl::cache<leaf_node, SL, std::hash<SL>,
            EXTRACT_POS_LEAF, std::equal_to<SL>> LEAF_CACHE;
    typedef typename LEAF_CACHE::hasher hasher_leaf;
    typedef typename LEAF_CACHE::key_equal key_equal_leaf;
    typedef ddl::cache<inner_node, SL, std::hash<SL>,
            EXTRACT_POS_INNER, std::equal_to<SL>> INNER_CACHE;
    typedef typename INNER_CACHE::hasher hasher_inner;
    typedef typename INNER_CACHE::key_equal key_equal_inner;
    LEAF_CACHE cache_leaf;
    INNER_CACHE cache_inner;

    typedef ddl::cache<std::pair<SL, data_type>, SL, std::hash<SL>,
            select1st<std::pair<SL, data_type>>, std::equal_to<SL>> DATA_CACHE;
    typedef typename DATA_CACHE::hasher hasher_data;
    typedef typename DATA_CACHE::key_equal key_equal_data;
    DATA_CACHE cache_data;
protected:
    void load_base(SL loc, base_ptr p) {
        node_visitor.read(loc, p);
    }
    NODE_TYPE load_base(SL loc) {
        auto tmp = (base_ptr)(operator new(sizeof(node)));
        node_visitor.read(loc, tmp);
        NODE_TYPE ans = tmp->_M_type;
        operator delete(tmp);
        return ans;
    }

    void fitsize_leaf() {
        if (cache_leaf.size() > cache_leaf.get_threshold()) {
            auto tmp = cache_leaf.begin();
            update_leaf(&(tmp->val));
            cache_leaf.erase(tmp);
        }
    }
    void fitsize_inner() {
        if (cache_inner.size() > cache_inner.get_threshold()) {
            auto tmp = cache_inner.begin();
            update_inner(&(tmp->val));
            cache_inner.erase(tmp);
        }
    }

    void fitsize_data() {
        if (cache_data.size() > cache_data.get_threshold()) {
            auto tmp = cache_data.begin();
            if (tmp->updated)
                update_data(tmp->val.first, tmp->val.second);
            cache_data.erase(tmp);
        }
    }
    loc_ptr save_data(const data_type& d) {
        return data_visitor.write(d);
    }
    loc_ptr reserve_data(const data_type& d) {
        loc_ptr loc = save_data(d);
        auto tmp = cache_data.insert(loc);
        tmp->val = std::pair<SL, data_type>(loc, d);
        fitsize_data();
        return loc;
    }
    void free_data(loc_ptr p) {
        data_visitor.deallocate(p);
    }
    void release_data(loc_ptr p) {
        auto t = cache_data.find(p);
        if (t != cache_data.end()) cache_data.erase(t);
        free_data(p);
    }
public:
    leaf_ptr request_leaf(SL loc) {
        auto t = cache_leaf.find(loc);
        if (t == cache_leaf.end()) {
            leaf_ptr tmp = &(cache_leaf.insert(loc)->val);
            load_leaf(loc, tmp);
            fitsize_leaf();
            return tmp;
        }
        return &(t->val);
    }
    inner_ptr request_inner(SL loc) {
        auto t = cache_inner.find(loc);
        if (t == cache_inner.end()) {
            inner_ptr tmp = &(cache_inner.insert(loc)->val);
            load_inner(loc, tmp);
            fitsize_inner();
            return tmp;
        }
        return &(t->val);
    }
    data_type* request_data(SL loc, bool adjust = false) {
        auto t = cache_data.find(loc);
        if (t == cache_data.end()) {
            auto tt = cache_data.insert(loc);
            tt->val.first = loc;
            data_type* tmp = &(tt->val.second);
            load_data(loc, tmp);
            fitsize_data();
            if (adjust) tt->updated = true;
            return tmp;
        }
        if (adjust) {t->updated = true;}
        return &(t->val.second);
    }
    void load_leaf(SL loc, leaf_ptr l) {
        node_visitor.read(loc, l);
    }
    void load_inner(SL loc, inner_ptr i) {
        node_visitor.read(loc, i);
    }
    void load_header() {
        node_visitor.head(&header);
    }
    void load_data(SL loc, data_type* d) {
        data_visitor.read(loc, d);
    }
    void update_leaf(leaf_ptr l) {
        node_visitor.write(l->pos, *l);
    }
    void update_inner(inner_ptr i) {
        node_visitor.write(i->pos, *i);
    }
    void update_data(loc_ptr loc, const data_type& d) {
        data_visitor.write(loc, d);
    }
    void update_header() {
        node_visitor.set_head(header);
    }
public:
    LRUBPTree(const std::string& ip, const std::string& dp, bool new_tree = false)
        : header(),
          cache_leaf(CACHE_INITIAL_SIZE_BLOCK, CACHE_MAX_SIZE_BLOCK,
                  hasher_leaf(), key_equal_leaf()),
          cache_inner(CACHE_INITIAL_SIZE_BLOCK, CACHE_MAX_SIZE_BLOCK,
                  hasher_inner(), key_equal_inner()),
          cache_data(CACHE_INNITIAL_SIZE_DATA, CACHE_MAX_SIZE_DATA,
                  hasher_data(), key_equal_data()) {
        if (new_tree) {
            node_visitor.initialize_path(ip);
            data_visitor.initialize_path(dp);
            node_visitor.set_head(header);
        }
        else {
            node_visitor.set_path(ip);
            data_visitor.set_path(dp);
            load_header();
        }
    }
    LRUBPTree(const std::string& ip, const std::string& dp,
            const key_compare& cmp, bool new_tree = false)
        : header(cmp),
          cache_leaf(CACHE_INITIAL_SIZE_BLOCK, CACHE_MAX_SIZE_BLOCK,
                  hasher_leaf(), key_equal_leaf()),
          cache_inner(CACHE_INITIAL_SIZE_BLOCK, CACHE_MAX_SIZE_BLOCK,
                  hasher_inner(), key_equal_inner()),
          cache_data(CACHE_INNITIAL_SIZE_DATA, CACHE_MAX_SIZE_DATA,
                  hasher_data(), key_equal_data()) {
        if (new_tree) {
            node_visitor.initialize_path(ip);
            data_visitor.initialize_path(dp);
            node_visitor.set_head(header);
        }
        else {
            node_visitor.set_path(ip);
            data_visitor.set_path(dp);
            load_header();
        }
    }
    ~LRUBPTree() {
        if (cache_inner.size()) {
            for (auto i = cache_inner.begin();
                 i != cache_inner.end();
                 i = i->later) {
                update_inner(&(i->val));
            }
            cache_inner.clear();
        }
        if (cache_leaf.size()) {
            for (auto i = cache_leaf.begin();
                 i != cache_leaf.end();
                 i = i->later) {
                update_leaf(&(i->val));
            }
            cache_leaf.clear();
        }
        if (cache_data.size()) {
            for (auto i = cache_data.begin();
                 i != cache_data.end();
                 i = i->later) {
                if (i->updated)
                    update_data(i->val.first, i->val.second);
            }
            cache_data.clear();
        }
        update_header();
        node_visitor.delink();
        data_visitor.delink();
    }
protected:
    loc_ptr new_leaf_block() {
        return node_visitor.write(leaf_node());
    }
    loc_ptr new_inner_block() {
        return node_visitor.write(inner_node());
    }
    void delete_block(base_ptr p) {
        node_visitor.deallocate(p->pos);
    }
    void destroy_block(base_ptr p) {
        if (p->_M_type==LEAF) {
            auto l = static_cast<leaf_ptr>(p);
            for (int i = 0; i < l->key_num; ++i) {
                free_data(l->data[i]);
            }
        }
        delete_block(p);
    }
    leaf_ptr get_leaf(SL loc) {
        leaf_ptr ans = &(cache_leaf.insert(loc)->val);
        ans->key_num = 0;
        ans->_M_type = LEAF;
        ans->pos = loc;
        ans->prev = 0;
        ans->next = 0;
        return ans;
    }
    inner_ptr get_inner(SL loc) {
        inner_ptr ans = &(cache_inner.insert(loc)->val);
        ans->key_num = 0;
        ans->_M_type = INNER;
        ans->pos = loc;
        return ans;
    }
    void destroy_node(base_ptr p) {
        if (p->_M_type==LEAF) {
            auto l = static_cast<leaf_ptr>(p);
            psgi::destroy(l);
            operator delete(l);
        }
        else {
            auto i = static_cast<inner_ptr>(p);
            psgi::destroy(i);
            operator delete(i);
        }
    }
protected:
    void split_leaf(leaf_ptr p, leaf_ptr& new_leaf) {
        int mid = p->key_num >> 1;
        SL nlb = new_leaf_block();
        new_leaf = get_leaf(nlb);
        new_leaf->next = p->next;

        std::uninitialized_copy(p->keys + mid,
                                p->keys + p->key_num, new_leaf->keys);
        psgi::destroy(p->keys + mid, p->keys + p->key_num);
        std::copy(p->data+mid, p->data+p->key_num, new_leaf->data);

        new_leaf->key_num = p->key_num - mid;
        p->key_num = mid;

        if (p->next==0) header._M_tail_leaf = new_leaf->pos;
        else {
            leaf_ptr tmp = request_leaf(new_leaf->next);
            tmp->prev = new_leaf->pos;
        }
        p->next = new_leaf->pos;
        new_leaf->prev = p->pos;
    }
    void split_inner(inner_ptr p, inner_ptr& new_inner, key_type& key_split, int slot_add) {
        int mid = p->key_num >> 1;
        bool odd_keys = p->key_num % 2;
        if (!odd_keys && slot_add < mid) --mid;
        SL nib = new_inner_block();
        new_inner = get_inner(nib);

        std::uninitialized_copy(p->keys + mid + 1,
                                p->keys + p->key_num, new_inner->keys);
        key_split = p->keys[mid];
        psgi::destroy(p->keys + mid + 1, p->keys + p->key_num);
        std::copy(p->child_id+mid+1, p->child_id + p->key_num + 1, new_inner->child_id);

        new_inner->key_num = p->key_num - (mid + 1);
        p->key_num = mid;
    }
    std::pair<iterator, bool>
    __insert_aux(base_ptr p, const key_type& key, const data_type& data,
                 key_type& key_split, loc_ptr& node_split) {
        if (p->_M_type==LEAF) {
            auto l = static_cast<leaf_ptr>(p);
            int slot_insert = __upper_key(l, key);
            if (slot_insert && key_equal(key, l->keys[slot_insert - 1]) ) {
                return std::pair<iterator, bool>(iterator(l, slot_insert-1, this), false);
            }
            leaf_ptr leaf_split = nullptr;
            if (l->is_full()) {
                split_leaf(l, leaf_split);
                node_split = leaf_split->pos;
                if (slot_insert > l->key_num) {
                    slot_insert -= l->key_num;
                    l = leaf_split;
                }
            }
            if (slot_insert < l->key_num) {
                psgi::construct(l->keys + l->key_num,
                                l->keys[l->key_num - 1]);
                std::copy_backward(l->keys + slot_insert,
                                   l->keys + l->key_num - 1,
                                   l->keys + l->key_num);
                l->keys[slot_insert] = key;
            }
            else {psgi::construct(l->keys + l->key_num, key);}
            std::copy_backward(l->data + slot_insert,
                               l->data + l->key_num,
                               l->data + l->key_num + 1);
            l->data[slot_insert] = reserve_data(data);
            ++l->key_num;
            if (leaf_split) {key_split = leaf_split->keys[0];}
            return std::pair<iterator, bool>(iterator(l, slot_insert, this), true);
        }
        else {
            auto i = static_cast<inner_ptr>(p);
            int slot_insert = __upper_key(i, key);
            key_type key_insert;
            loc_ptr child_insert = 0;
            base_ptr child;
            loc_ptr id = i->child_id[slot_insert];
            if (load_base(id)==LEAF) child = request_leaf(id);
            else child = request_inner(id);
            std::pair<iterator, bool>
                    ans = __insert_aux(child, key, data,key_insert, child_insert);
            if (child_insert) {
                inner_ptr inner_split = nullptr;
                if (i->is_full()) {
                    split_inner(i, inner_split, key_split, slot_insert);
                    node_split = inner_split->pos;
                    if (slot_insert > i->key_num) {
                        slot_insert -= (i->key_num + 1);
                        i = inner_split;
                    }
                }
                if (slot_insert < i->key_num) {
                    psgi::construct(i->keys + i->key_num,
                                    i->keys[i->key_num - 1]);
                    std::copy_backward(i->keys + slot_insert,
                                       i->keys + i->key_num - 1,
                                       i->keys + i->key_num);
                    std::copy_backward(i->child_id+slot_insert+1,
                                       i->child_id + i->key_num + 1,
                                       i->child_id + i->key_num + 2);
                    i->keys[slot_insert] = key_insert;
                }
                else {psgi::construct(i->keys + i->key_num, key_insert);}
                i->child_id[slot_insert+1] = child_insert;
                ++i->key_num;
            }
            return ans;
        }
    }
    std::pair<iterator, bool> __insert(const key_type& key, const data_type& data) {
        if (header._M_root==0) {
            header._M_root = new_leaf_block();
            header._M_head_leaf = header._M_root;
            header._M_tail_leaf = header._M_root;
            header._M_root_type = LEAF;
            get_leaf(header._M_root);
        }

        loc_ptr root_split = 0;
        key_type key_split;
        std::pair<iterator, bool>
                ans = __insert_aux(root(), key, data, key_split, root_split);

        if (root_split) {
            SL nir = new_inner_block();
            inner_ptr new_root = get_inner(nir);
            new_root->key_num = 1;
            psgi::construct(new_root->keys, key_split);
            new_root->child_id[0] = header._M_root;
            new_root->child_id[1] = root_split;
            header._M_root = new_root->pos;
            header._M_root_type = INNER;
        }

        if (ans.second) {++header._M_item_count;}
        return ans;
    }
protected:
    struct PERCOLATE_PARAMETERS {
        bool ADJUST;
        bool DELETE;
        key_type key_second;
        key_type key_first;
        bool adjust_offset;
        bool adjust_both;
        bool delete_offset;

        PERCOLATE_PARAMETERS()
                : ADJUST(0), DELETE(0),
                  key_second(key_type()), key_first(key_type()),
                  adjust_offset(0), adjust_both(0), delete_offset(0) {}
        ~PERCOLATE_PARAMETERS() {
            typedef typename std::is_trivially_destructible<key_type> trivial_destructor;
            if (trivial_destructor() == std::false_type()) {
                psgi::destroy(&key_second);
                psgi::destroy(&key_first);
            }
        }
        PERCOLATE_PARAMETERS(const PERCOLATE_PARAMETERS& oth)
                : ADJUST(oth.ADJUST), DELETE(oth.DELETE),
                  key_second(oth.key_second),
                  key_first(oth.key_first),
                  adjust_offset(oth.adjust_offset),
                  adjust_both(oth.adjust_both),
                  delete_offset(oth.delete_offset) {}
        PERCOLATE_PARAMETERS&
        operator=(const PERCOLATE_PARAMETERS& oth) {
            if (this != &oth) {
                ADJUST = oth.ADJUST;
                DELETE = oth.DELETE;
                key_second = oth.key_second;
                key_first = oth.key_first;
                adjust_offset = oth.adjust_offset;
                adjust_both = oth.adjust_both;
                delete_offset = oth.delete_offset;
            }
            return *this;
        }
    };
    typedef PERCOLATE_PARAMETERS PP;
    struct FAMILY {
        loc_ptr left_sibling;
        loc_ptr right_sibling;
        key_type min_key;
        key_type next_min_key;

        FAMILY()
                : left_sibling(0), right_sibling(0),
                  min_key(), next_min_key() {}
        FAMILY(loc_ptr ls, loc_ptr rs)
                : left_sibling(ls), right_sibling(rs),
                  min_key(), next_min_key() {}
        ~FAMILY() {
            typedef typename std::is_trivially_destructible<key_type> trivial_destructor;
            if (trivial_destructor() == std::false_type()) {
                psgi::destroy(&min_key);
                psgi::destroy(&next_min_key);
            }
        }
        FAMILY(const FAMILY& oth)
                : left_sibling(oth.left_sibling),
                  right_sibling(oth.right_sibling),
                  min_key(oth.min_key),
                  next_min_key(oth.next_min_key) {}
        FAMILY& operator=(const FAMILY& oth) {
            if (this != &oth) {
                left_sibling = oth.left_sibling;
                right_sibling = oth.right_sibling;
                min_key = oth.min_key;
                next_min_key = oth.next_min_key;
            }
            return *this;
        }
    };
    PP __erase(base_ptr p, const key_type& key, const FAMILY& fa) {
        PP result;
        if (p->_M_type==LEAF) {
            auto l = static_cast<leaf_ptr>(p);
            int slot_erase = __lower_key(l, key);
            if (slot_erase>=l->key_num
                || key_comp()(key, l->keys[slot_erase])) return PP();
            --l->key_num;
            release_data(l->data[slot_erase]);
            if (l->is_few()) {
                if (fa.left_sibling!=0) {
                    leaf_ptr ls = request_leaf(fa.left_sibling);
                    if (l->key_num + ls->key_num
                        > leaf_node::maxleafslots) {
                        ++l->key_num;
                        std::copy_backward(l->keys,
                                           l->keys + slot_erase,
                                           l->keys + slot_erase + 1);
                        std::copy_backward(l->data,
                                           l->data+slot_erase,
                                           l->data+slot_erase+1);
                        l->keys[0] = ls->keys[ls->key_num - 1];
                        l->data[0] = ls->data[ls->key_num - 1];

                        psgi::destroy(ls->keys + ls->key_num - 1);
                        --ls->key_num;

                        result.ADJUST = true;
                        result.key_first = l->keys[0];
                    }
                    else {
                        std::uninitialized_copy(l->keys,
                                                l->keys + slot_erase,
                                                ls->keys + ls->key_num);
                        std::copy(l->data, l->data+slot_erase, ls->data+ls->key_num);
                        ls->key_num += slot_erase;
                        std::uninitialized_copy(l->keys + slot_erase + 1,
                                                l->keys + l->key_num + 1,
                                                ls->keys + ls->key_num);
                        std::copy(l->data+slot_erase+1,
                                  l->data + l->key_num + 1,
                                ls->data+ls->key_num);
                        ls->key_num += l->key_num - slot_erase;

                        ls->next = l->next;
                        if (l->next!=0) {
                            leaf_ptr tmp = request_leaf(l->next);
                            tmp->prev = ls->pos;
                        }
                        else header._M_tail_leaf = ls->pos;
                        delete_block(l);
                        cache_leaf.erase(l->pos);

                        result.DELETE = true;
                    }
                }
                else if (fa.right_sibling!=0) {
                    leaf_ptr rs = request_leaf(fa.right_sibling);
                    if (l->key_num + rs->key_num
                        > leaf_node::maxleafslots) {
                        result.ADJUST = true;
                        result.adjust_offset = true;
                        result.key_second = rs->keys[1];
                        if (!slot_erase) {
                            result.adjust_both = true;
                            result.key_first = l->keys[1];
                        }

                        ++l->key_num;
                        std::copy(l->keys + slot_erase + 1,
                                  l->keys + l->key_num,
                                  l->keys + slot_erase);
                        std::copy(l->data+slot_erase+1,
                                  l->data+l->key_num,
                                  l->data+slot_erase);
                        l->keys[l->key_num - 1] = rs->keys[0];
                        l->data[l->key_num - 1] = rs->data[0];

                        std::copy(rs->keys + 1,
                                  rs->keys + rs->key_num,
                                  rs->keys);
                        std::copy(rs->data+1,
                                  rs->data+rs->key_num,
                                  rs->data);
                        psgi::destroy(rs->keys + rs->key_num - 1);
                        --rs->key_num;
                    }
                    else {
                        std::copy(l->keys + slot_erase + 1,
                                  l->keys + l->key_num + 1,
                                  l->keys + slot_erase);
                        psgi::destroy(l->keys + l->key_num);
                        std::copy(l->data+slot_erase+1,
                                  l->data + l->key_num + 1,
                                  l->data+slot_erase);
                        std::uninitialized_copy(rs->keys,
                                                rs->keys + rs->key_num,
                                                l->keys + l->key_num);
                        std::copy(rs->data, rs->data+rs->key_num, l->data + l->key_num);
                        l->key_num += rs->key_num;

                        l->next = rs->next;
                        if (rs->next!=0) {
                            leaf_ptr tmp = request_leaf(rs->next);
                            tmp->prev = l->pos;
                        }
                        else header._M_tail_leaf = l->pos;
                        delete_block(rs);
                        cache_leaf.erase(rs->pos);

                        result.DELETE = true;
                        result.delete_offset = true;
                    }
                }
                else if (root()->key_num == 0) {
                    auto rot = static_cast<leaf_ptr>(root());
                    ++rot->key_num;
                    delete_block(rot);
                    cache_leaf.erase(rot->pos);
                    header._M_root = 0;
                    header._M_head_leaf = 0;
                    header._M_tail_leaf = 0;
                    header._M_item_count = 0;
                    header._M_root_type = LEAF;
                    return PP();
                }
                else {
                    std::copy(l->keys + slot_erase + 1,
                              l->keys + l->key_num + 1,
                              l->keys + slot_erase);
                    psgi::destroy(l->keys + l->key_num);
                    std::copy(l->data+slot_erase+1,
                              l->data + l->key_num + 1,
                              l->data+slot_erase);
                }
            }
            else {
                if (slot_erase==0) {
                    result.ADJUST = true;
                    result.key_first = l->keys[1];
                }
                std::copy(l->keys + slot_erase + 1,
                          l->keys + l->key_num + 1,
                          l->keys + slot_erase);
                psgi::destroy(l->keys + l->key_num);
                std::copy(l->data+slot_erase+1,
                          l->data + l->key_num + 1,
                          l->data+slot_erase);
            }
            --header._M_item_count;
        }
        else {
            auto i = static_cast<inner_ptr>(p);
            int slot_erase = __upper_key(i, key);

            loc_ptr lc;
            if (slot_erase > 0) {
                lc = i->child_id[slot_erase-1];
            }
            else {
                if (fa.left_sibling) {
                    inner_ptr ls = request_inner(fa.left_sibling);
                    lc = ls->child_id[ls->key_num];
                }
                else lc = 0;
            }
            loc_ptr rc;
            if (slot_erase < i->key_num) {
                rc = i->child_id[slot_erase+1];
            }
            else {
                if (fa.right_sibling) {
                    inner_ptr rs = request_inner(fa.right_sibling);
                    rc = rs->child_id[0];
                }
                else rc = 0;
            }
            FAMILY PASS(lc, rc);
            PASS.min_key = (slot_erase==0 ? fa.min_key : i->keys[slot_erase - 1]);
            if (slot_erase==0) {PASS.next_min_key = i->keys[0];}

            base_ptr child;
            loc_ptr id = i->child_id[slot_erase];
            if (load_base(id)==LEAF) child = request_leaf(id);
            else child = request_inner(id);
            PP ans = __erase(child, key, PASS);

            if (ans.ADJUST) {
                int slot_adjust = slot_erase + ans.adjust_offset;
                if (!slot_adjust) {
                    result.ADJUST = true;
                    result.key_first = ans.key_first;
                }
                else {
                    i->keys[slot_adjust - 1]
                            = (ans.adjust_offset ? ans.key_second : ans.key_first);
                }
                if (ans.adjust_both) {
                    if (!slot_erase) {
                        result.ADJUST = true;
                        result.key_first = ans.key_first;
                    }
                    else {
                        i->keys[slot_erase - 1]= ans.key_first;
                    }
                }
            }
            if (ans.DELETE) {
                --i->key_num;
                int slot_delete = slot_erase + ans.delete_offset;
                if (i->is_few()) {
                    if (fa.left_sibling!=0) {
                        inner_ptr ls = request_inner(fa.left_sibling);
                        if (i->key_num + ls->key_num
                            > inner_node::maxinnerslots-2) {
                            result.ADJUST = true;
                            result.key_first = ls->keys[ls->key_num - 1];

                            ++i->key_num;
                            if (slot_delete) {
                                std::copy_backward(i->keys,
                                                   i->keys + slot_delete - 1,
                                                   i->keys + slot_delete);
                                std::copy_backward(i->child_id,
                                                   i->child_id+slot_delete,
                                                   i->child_id+slot_delete+1);
                                i->keys[0] = fa.min_key;
                            }

                            i->child_id[0] = ls->child_id[ls->key_num];

                            psgi::destroy(ls->keys + ls->key_num - 1);
                            --ls->key_num;
                        }
                        else {
                            if (slot_delete) {
                                std::copy(i->child_id,
                                          i->child_id+slot_delete,
                                          ls->child_id+ls->key_num);
                                psgi::construct(ls->keys + ls->key_num, fa.min_key);
                                std::uninitialized_copy(i->keys,
                                                        i->keys + slot_delete - 1,
                                                        ls->keys + ls->key_num + 1);
                                ls->key_num += slot_delete;
                            }
                            std::uninitialized_copy(i->keys + slot_delete,
                                                    i->keys + i->key_num + 1,
                                                    ls->keys + ls->key_num);
                            std::copy(i->child_id+slot_delete+1,
                                      i->child_id + i->key_num + 2,
                                      ls->child_id + ls->key_num + 1);
                            ls->key_num += i->key_num - slot_delete;

                            delete_block(i);
                            cache_inner.erase(i->pos);

                            result.DELETE = true;
                        }
                    }
                    else if (fa.right_sibling!=0) {
                        inner_ptr rs = request_inner(fa.right_sibling);
                        if (i->key_num + rs->key_num
                            > inner_node::maxinnerslots-2) {
                            result.ADJUST = true;
                            result.adjust_offset = true;
                            result.key_second = fa.next_min_key;

                            ++i->key_num;
                            if (slot_delete) {
                                std::copy(i->keys + slot_delete,
                                          i->keys + i->key_num,
                                          i->keys + slot_delete - 1);
                            }
                            else {
                                result.adjust_both = true;
                                result.key_first = i->keys[0];
                                std::copy(i->keys + 1,
                                          i->keys + i->key_num,
                                          i->keys);
                            }
                            std::copy(i->child_id+slot_delete+1,
                                      i->child_id + i->key_num + 1,
                                      i->child_id+slot_delete);
                            i->keys[i->key_num - 1] = fa.next_min_key;
                            i->child_id[i->key_num] = rs->child_id[0];

                            std::copy(rs->keys + 1,
                                      rs->keys + rs->key_num,
                                      rs->keys);
                            std::copy(rs->child_id+1,
                                      rs->child_id + rs->key_num + 1,
                                      rs->child_id);
                            psgi::destroy(rs->keys + rs->key_num - 1);
                            --rs->key_num;
                        }
                        else {
                            if (slot_delete) {
                                std::copy(i->keys + slot_delete,
                                          i->keys + i->key_num + 1,
                                          i->keys + slot_delete - 1);
                            }
                            else {
                                result.ADJUST = true;
                                result.key_first = i->keys[0];
                                std::copy(i->keys + 1,
                                          i->keys + i->key_num + 1,
                                          i->keys);
                            }
                            i->keys[i->key_num] = fa.next_min_key;
                            std::copy(i->child_id+slot_delete+1,
                                      i->child_id + i->key_num + 2,
                                      i->child_id+slot_delete);
                            std::uninitialized_copy(rs->keys,
                                                    rs->keys + rs->key_num,
                                                    i->keys + i->key_num + 1);
                            std::copy(rs->child_id, rs->child_id + rs->key_num + 1,
                                      i->child_id + i->key_num + 1);
                            i->key_num += rs->key_num;

                            delete_block(rs);
                            cache_inner.erase(rs->pos);

                            result.DELETE = true;
                            result.delete_offset = true;
                        }
                    }
                    else if (root()->key_num == 0) {
                        auto rot = static_cast<inner_ptr>(root());
                        ++rot->key_num;
                        int n = (slot_delete ? 0 : 1);
                        loc_ptr new_root_loc = rot->child_id[n];
                        delete_block(rot);
                        cache_inner.erase(rot->pos);
                        header._M_root = new_root_loc;
                        header._M_root_type = load_base(new_root_loc);
                        return PP();
                    }
                    else {
                        if (!slot_delete) {
                            std::copy(i->keys + 1,
                                      i->keys + i->key_num + 1,
                                      i->keys);
                        }
                        else {
                            std::copy(i->keys + slot_delete,
                                      i->keys + i->key_num + 1,
                                      i->keys + slot_delete - 1);
                        }
                        psgi::destroy(i->keys + i->key_num);
                        std::copy(i->child_id+slot_delete+1,
                                  i->child_id + i->key_num + 2,
                                  i->child_id+slot_delete);
                    }
                }
                else {
                    if (!slot_delete) {
                        result.ADJUST = true;
                        result.key_first = i->keys[0];
                        std::copy(i->keys + 1,
                                  i->keys + i->key_num + 1,
                                  i->keys);
                    }
                    else {
                        std::copy(i->keys + slot_delete,
                                  i->keys + i->key_num + 1,
                                  i->keys + slot_delete - 1);
                    }
                    psgi::destroy(i->keys + i->key_num);
                    std::copy(i->child_id+slot_delete+1,
                              i->child_id + i->key_num + 2,
                              i->child_id+slot_delete);
                }
            }
        }
        return result;
    }
    void __clear_loop(base_ptr p) {
        if (p->_M_type==INNER) {
            auto i = static_cast<inner_ptr>(p);
            for (size_t s = 0, st = i->key_num + 1; s < st; ++s) {
                auto tmp = (base_ptr)(operator new(sizeof(node)));
                load_base(i->child_id[s], tmp);
                __clear_loop(tmp);
            }
        }
        destroy_block(p);
        destroy_node(p);
    }
protected:
    template <class node_type>
    int __lower_key(const node_type* p, const key_type& k) const {
        if (p->key_num == 0) return 0;
        int l = 0, r = p->key_num;
        while (l < r) {
            int mid = (l + r) >> 1;
            if (!key_comp()(p->keys[mid], k)) r = mid;
            else l = mid + 1;
        }
        return l;
    }
    template <class node_type>
    int __upper_key(const node_type* p, const key_type& k) const {
        if (p->key_num == 0) return 0;
        int l = 0, r = p->key_num;
        while (l < r) {
            int mid = (l + r) >> 1;
            if (key_comp()(k, p->keys[mid])) r = mid;
            else l = mid + 1;
        }
        return l;
    }

    iterator __find(base_ptr p, const key_type& k) {
        if (p->_M_type==INNER) {
            auto i = static_cast<inner_node*>(p);
            int s = __upper_key(i, k);
            loc_ptr id = i->child_id[s];
            base_ptr c;
            if (load_base(id)==LEAF) c = request_leaf(id);
            else c = request_inner(id);
            return __find(c, k);
        }
        else {
            auto l = static_cast<leaf_ptr>(p);
            int s = __lower_key(l, k);
            if (s < l->key_num && key_equal(k, l->keys[s]))
                return iterator(l, s, this);
            else return end();
        }
    }
    iterator __lower_bound(base_ptr p, const key_type& k) {
        iterator ans;
        if (p->_M_type==INNER) {
            auto i = static_cast<inner_node*>(p);
            int s = __upper_key(i, k);
            loc_ptr id = i->child_id[s];
            base_ptr c;
            if (load_base(id)==LEAF) c = request_leaf(id);
            else c = request_inner(id);
            ans = __lower_bound(c, k);
            if (s < i->key_num && ans == end()) {
                base_ptr d;
                loc_ptr t = i->child_id[s+1];
                if (load_base(t)==LEAF) d = request_leaf(t);
                else d = request_inner(t);
                ans = __find(d, i->keys[s]);
            }
        }
        else {
            auto l = static_cast<leaf_ptr>(p);
            int s = __lower_key(l, k);
            if (s < l->key_num)
                ans = iterator(l, s, this);
            else ans = end();
        }
        return ans;
    }
    iterator __upper_bound(base_ptr p, const key_type& k) {
        iterator ans;
        if (p->_M_type==INNER) {
            auto i = static_cast<inner_node*>(p);
            int s = __upper_key(i, k);
            loc_ptr id = i->child_id[s];
            base_ptr c;
            if (load_base(id)==LEAF) c = request_leaf(id);
            else c = request_inner(id);
            ans = __upper_bound(c, k);
            if (s < i->key_num && ans == end())
                ans = __find(i, i->keys[s]);
        }
        else {
            auto l = static_cast<leaf_ptr>(p);
            int s = __upper_key(l, k);
            if (s < l->key_num)
                ans = iterator(l, s, this);
            else ans = end();
        }
        return ans;
    }
public:
    iterator begin() {
        if (header._M_head_leaf) {
            leaf_ptr tmp = request_leaf(header._M_head_leaf);
            iterator ans(tmp, 0, this);
            return ans;
        }
        else {
            return iterator(0, 0, this);
        }
    }
    iterator end() {
        if (header._M_tail_leaf) {
            leaf_ptr tmp = request_leaf(header._M_tail_leaf);
            iterator ans(tmp, tmp->key_num, this);
            return ans;
        }
        else {
            return iterator(0, 0, this);
        }
    }

    size_type size() const {return header._M_item_count;}
    bool empty() const {return size() == (size_type)0;}

    iterator find(const key_type& k) {
        base_ptr p = root();
        if (!p) return end();
        return __find(p, k);
        /*
        while (p->_M_type==INNER) {
            auto i = static_cast<const inner_node*>(p);
            int s = __upper_key(i, k);
            loc_ptr id = i->child_id[s];
            if (load_base(id)==LEAF) {
                p = request_leaf(id);
            }
            else {
                p = request_inner(id);
            }
        }
        auto l = static_cast<leaf_ptr>(p);
        int s = __lower_key(l, k);
        if (s < l->key_num && key_equal(k, l->keys[s]) ) {
            iterator ans(l, s, this);
            return ans;
        }
        else {
            return end();
        }
         */
    }
    iterator lower_bound(const key_type& k) {
        base_ptr p = root();
        if (!p) {return end();}
        return __lower_bound(p, k);
    }
    iterator upper_bound(const key_type& k) {
        base_ptr p = root();
        if (!p) return end();
        return __upper_bound(p, k);
    }
    data_type& operator[](const key_type& k) {
        iterator i = lower_bound(k);
        if (i == end() || i.curslot == i.curleaf->key_num || key_comp()(k, i.key())) {
            i = insert(k, data_type()).first;
        }
        return i.data(true);
    }

    std::pair<iterator, bool> insert(const value_type& v) {
        return __insert(v.first, v.second);
    }
    std::pair<iterator, bool> insert(const key_type& key, const data_type& x) {
        return __insert(key, x);
    }
    void erase(const key_type& k) {
        __erase(root(), k, FAMILY(0, 0));
    }
    void erase(iterator it) {
        __erase(root(), it.key(), FAMILY(0, 0));
    }

    void clear() {
        if (header._M_root) {
            __clear_loop(root());
            header._M_root = 0;
            header._M_item_count = 0;
            header._M_head_leaf = 0;
            header._M_tail_leaf = 0;
            header._M_root_type = LEAF;
        }
    }
    void clean() {
        cache_inner.clear();
        cache_leaf.clear();
        cache_data.clear();

        std::string node_path = node_visitor.get_path();
        node_visitor.delete_file();
        std::string data_path = data_visitor.get_path();
        data_visitor.delete_file();
        node_visitor.initialize_path(node_path);
        data_visitor.initialize_path(data_path);

        header._M_root = 0;
        header._M_item_count = 0;
        header._M_head_leaf = 0;
        header._M_tail_leaf = 0;
        header._M_root_type = LEAF;
        node_visitor.set_head(header);
    }
};


#endif