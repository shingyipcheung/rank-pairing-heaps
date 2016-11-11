/*
The MIT License (MIT)
Copyright (c) 2016 James Yip
Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
#ifndef _RP_HEAP_H_
#define _RP_HEAP_H_

// #include <assert.h>
#include <algorithm>
#include <stdexcept>
#include <vector>
#include <stack>

template <class _Ty>
struct _Node
{
    typedef _Node* _Nodeptr;
    _Node(const _Ty& right) : _Val(right)
    {
        _Left = _Next = _Parent = nullptr;
        _Rank = 0;
    }
    _Node(_Ty&& _Val) : _Val(std::move(_Val))
    {
        _Left = _Next = _Parent = nullptr;
        _Rank = 0;
    }
    _Ty _Val;
    _Nodeptr _Left, _Next, _Parent;
    int _Rank;
private:
    _Node& operator=(const _Node&);
};

template <class _Myheap>
class _Iterator
{
public:
    friend _Myheap;
    typedef typename _Myheap::_Nodeptr _Nodeptr;
    typedef typename _Myheap::value_type value_type;
    typedef typename _Myheap::difference_type difference_type;
    typedef typename _Myheap::const_reference const_reference;
    typedef typename _Myheap::const_pointer const_pointer;

    _Iterator(_Nodeptr _Ptr = nullptr)
    {
        this->_Ptr = _Ptr;
    }
    const_reference operator*() const
    {
        return this->_Ptr->_Val;
    }
    const_pointer operator->() const
    {
        return &(operator*());
    }
    _Nodeptr _Ptr;
};

template <class _Ty, class _Pr = less<_Ty>, class _Alloc = std::allocator<_Ty>>
class rp_heap
{
public:
    typedef rp_heap<_Ty, _Pr, _Alloc> _Myt;
    typedef _Node<_Ty> _Node;
    typedef _Node* _Nodeptr;

    typedef _Pr key_compare;

    typedef _Alloc allocator_type;
    typedef typename _Alloc::template rebind<_Node>::other _Alty;
    typedef typename _Alloc::value_type value_type;
    typedef typename _Alloc::pointer pointer;
    typedef typename _Alloc::const_pointer const_pointer;
    typedef typename _Alloc::reference reference;
    typedef typename _Alloc::const_reference const_reference;
    typedef typename _Alloc::difference_type difference_type;
    typedef typename _Alloc::size_type size_type;

    typedef _Iterator<_Myt> const_iterator;

    rp_heap(const _Pr& _Pred = _Pr()) : comp(_Pred)
    {
        _Mysize = 0;
        _Myhead = nullptr;
    }

    ~rp_heap()
    {
        clear();
    }

    bool empty() const
    {
        return _Mysize == 0;
    }

    size_type size() const
    {
        return _Mysize;
    }

    const_reference top() const
    {
        return _Myhead->_Val;
    }

    const_iterator push(const value_type& _Val)
    {
        _Nodeptr _Ptr = _Alnod.allocate(1);
        _Alnod.construct(_Ptr, _Val);
        _Insert_root(_Ptr);
        _Mysize++;
        return const_iterator(_Ptr);
    }

    const_iterator push(value_type&& x)
    {
        _Nodeptr _Ptr = _Alnod.allocate(1);
        _Alnod.construct(_Ptr, std::forward<value_type>(x));
        _Insert_root(_Ptr);
        _Mysize++;
        return const_iterator(_Ptr);
    }

    void pop() //delete min
    {
        if (empty())
            throw std::runtime_error("pop error: empty heap");
        std::vector<_Nodeptr> _Bucket(_Max_bucket_size(), nullptr);
        // assert_children(_MinRoot);
        for (_Nodeptr _Ptr = _Myhead->_Left; _Ptr; )
        {
            _Nodeptr _NextPtr = _Ptr->_Next;
            _Ptr->_Next = nullptr;
            _Ptr->_Parent = nullptr;
            _Multipass(_Bucket, _Ptr);
            _Ptr = _NextPtr;
        }
        for (_Nodeptr _Ptr = _Myhead->_Next; _Ptr != _Myhead; )
        {
            _Nodeptr _NextPtr = _Ptr->_Next;
            _Ptr->_Next = nullptr;
            _Multipass(_Bucket, _Ptr);
            _Ptr = _NextPtr;
        }
        _Freenode(_Myhead);
        _Myhead = nullptr;
        std::for_each(_Bucket.begin(), _Bucket.end(), [&](_Nodeptr _Ptr)
        {
            if (_Ptr)
                _Insert_root(_Ptr);
        });
    }

    void pop(value_type& _Val)
    {
        if (empty())
            throw std::runtime_error("pop error: empty heap");
        _Val = std::move(_Myhead->_Val);
        pop();
    }

    void clear()
    {
        // while (!empty())
        //     pop();
        // post order traversal using two stacks
        if (!empty())
        {
            std::stack<_Nodeptr> _Stack_in, _Stack_out;
            _Stack_in.push(_Myhead);
            while (!_Stack_in.empty())
            {
                _Nodeptr _Ptr = _Stack_in.top();
                _Stack_in.pop();
                _Stack_out.push(_Ptr);
                if (_Ptr->_Left)
                    _Stack_in.push(_Ptr->_Left);
                if (_Ptr->_Next && _Ptr->_Next != _Myhead)
                    _Stack_in.push(_Ptr->_Next);
            }
            while (!_Stack_out.empty())
            {
                _Nodeptr _Ptr = _Stack_out.top();
                _Freenode(_Ptr);
                _Stack_out.pop();
            }
        }
        _Myhead = nullptr;
        //assert(empty());
    }

    void decrease(const_iterator _It, const value_type& _Val)
    {
        _Nodeptr _Ptr = _It._Ptr;
        if (comp(_Val, _Ptr->_Val))
            _Ptr->_Val = _Val;
        if (_Ptr == _Myhead)
            return;
        if (_Ptr->_Parent == nullptr) //one of the roots
        {
            if (comp(_Ptr->_Val, _Myhead->_Val))
                _Myhead = _Ptr;
        }
        else
        {
            _Nodeptr _ParentPtr = _Ptr->_Parent;
            if (_Ptr == _ParentPtr->_Left)
            {
                _ParentPtr->_Left = _Ptr->_Next;
                if (_ParentPtr->_Left)
                    _ParentPtr->_Left->_Parent = _ParentPtr;
            }
            else
            {
                _ParentPtr->_Next = _Ptr->_Next;
                if (_ParentPtr->_Next)
                    _ParentPtr->_Next->_Parent = _ParentPtr;
            }
            // assert_children(_ParentPtr);
            _Ptr->_Next = _Ptr->_Parent = nullptr;
            _Ptr->_Rank = (_Ptr->_Left) ? _Ptr->_Left->_Rank + 1 : 0;
            // assert_half_tree(_Ptr);
            _Insert_root(_Ptr);
            //type-2 rank reduction
            if (_ParentPtr->_Parent == nullptr) // is a root
                _ParentPtr->_Rank = (_ParentPtr->_Left) ? _ParentPtr->_Left->_Rank + 1 : 0;
            else
            {
                while (_ParentPtr->_Parent)
                {
                    int i = _ParentPtr->_Left ? _ParentPtr->_Left->_Rank : -1;
                    int j = _ParentPtr->_Next ? _ParentPtr->_Next->_Rank : -1;
#ifdef TYPE1_RANK_REDUCTION
                    int k = (i != j) ? max(i, j) : i + 1; //type-1 rank reduction
#else
                    int k = (abs(i - j) > 1) ? max(i, j) : max(i, j) + 1; //type-2 rank reduction
#endif // TYPE1_RANK_REDUCTION
                    if (k >= _ParentPtr->_Rank)
                        break;
                    _ParentPtr->_Rank = k;
                    _ParentPtr = _ParentPtr->_Parent;
                }
            }
        }
    }

private:

    void _Freenode(_Nodeptr _Ptr)
    {
        _Alnod.destroy(_Ptr);
        _Alnod.deallocate(_Ptr, 1);
        _Mysize--;
    }

    // void assert_half_tree(_Nodeptr _Ptr)
    // {
    //     assert(_Ptr->_Next == nullptr && _Ptr->_Parent == nullptr);
    // }

    // void assert_parent(_Nodeptr _Ptr)
    // {
    //     assert(_Ptr->_Parent->_Left == _Ptr || _Ptr->_Parent->_Next == _Ptr);
    // }

    // void assert_children(_Nodeptr _Ptr)
    // {
    //     if (_Ptr->_Left)
    //         assert(_Ptr->_Left->_Parent == _Ptr);
    //     if (_Ptr->_Next)
    //         assert(_Ptr->_Next->_Parent == _Ptr);
    // }

    void _Insert_root(_Nodeptr _Ptr)
    {
        if (_Myhead == nullptr)
        {
            _Myhead = _Ptr;
            _Ptr->_Next = _Ptr;
        }
        else
        {
            _Ptr->_Next = _Myhead->_Next;
            _Myhead->_Next = _Ptr;
            if (comp(_Ptr->_Val, _Myhead->_Val))
                _Myhead = _Ptr;
        }
    }

    _Nodeptr _Link(_Nodeptr _Left, _Nodeptr _Right)
    {
        if (_Right == nullptr)
            return _Left;
        // assert_half_tree(_Left);
        // assert_half_tree(_Right);
        _Nodeptr _Winner, _Loser;
        if (comp(_Right->_Val, _Left->_Val))
        {
            _Winner = _Right;
            _Loser = _Left;
        }
        else
        {
            _Winner = _Left;
            _Loser = _Right;
        }
        _Loser->_Parent = _Winner;
        if (_Winner->_Left)
        {
            _Loser->_Next = _Winner->_Left;
            _Loser->_Next->_Parent = _Loser;
        }
        _Winner->_Left = _Loser;
        _Winner->_Rank = _Loser->_Rank + 1;
        // assert_children(_Winner);
        // assert_parent(_Loser);
        // assert_half_tree(_Winner);
        return _Winner;
    }

    inline size_type _Max_bucket_size() //ceil(log2(size)) + 1
    {
        size_type _Bit = 1, _Count = _Mysize;
        while (_Count >>= 1)
            _Bit++;
        return _Bit + 1;
    }

    template <class _Container>
    void _Multipass(_Container& _Bucket, _Nodeptr _Ptr)
    {
        // if ((size_t)_Ptr->_Rank >= _Bucket.size())
        // {
        //     _Bucket.resize(_Ptr->_Rank + 1, nullptr);
        // }
        // else
        // {
        while (_Bucket[_Ptr->_Rank] != nullptr)
        {
            unsigned int _Rank = _Ptr->_Rank;
            _Ptr = _Link(_Ptr, _Bucket[_Rank]);
            // assert_children(_Ptr);
            // assert_half_tree(_Ptr);
            _Bucket[_Rank] = nullptr;
            // if ((size_t)_Ptr->_Rank >= _Bucket.size())
            // {
            //     _Bucket.resize(_Ptr->_Rank + 1, nullptr);
            //     break;
            // }
        }
        // }
        _Bucket[_Ptr->_Rank] = _Ptr;
    }

    _Pr comp;
    _Nodeptr _Myhead;
    size_type _Mysize;
    _Alty _Alnod;
};

#endif /* _RP_HEAP_H_ */
