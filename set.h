////////////////////////////////////////////////////////////////////////////////
//  $Id: set.h,v 1.1 2006/01/27 23:09:56 dmouldin Exp $
//
//  Visual Leak Detector (Version 1.0)
//  Copyright (c) 2005 Dan Moulding
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU Lesser General Public License as published by
//  the Free Software Foundation; either version 2.1 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//  See COPYING.txt for the full terms of the GNU Lesser General Public License.
//
////////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef VLDBUILD
#error "This header should only be included by Visual Leak Detector when building it from source. Applications should never include this header."
#endif

#include "tree.h"    // Provides access to the Tree template class.
#include "vldheap.h" // Provides internal new and delete operators.

////////////////////////////////////////////////////////////////////////////////
//
//  The Set Template Class
//
//  This is a STL-like set template. It makes use of the Tree class template to
//  enable fast insert, find, and erase operations.
//
//  Note that while this is a STL-like class, it is not a full STL-compliant
//  implementation of the STL set container. It contains just the bare minimum
//  functionality required by Visual Leak Detector. Because of its "lightweight"
//  nature, this set class has a noticeable performance advantage over most
//  standard STL set implementations.
//
template <typename Tk>
class Set {
public:
    class Iterator {
    public:
        // Constructor
        Iterator ()
        {
            // Plainly constructed iterators don't reference anything.
            m_node = NULL;
            m_tree = NULL;
        }

        // operator != - Inequality operator for Set Iterators. Two Set
        //   Iterators are considered equal if and only if they both reference
        //   the same key in the same Set.
        //
        //  - other (IN): The other Set Iterator to compare against.
        //
        //  Return Value:
        //
        //    Returns true if the specified Set Iterator is not equal to this
        //    Set Iterator; otherwise, returns false.
        //
        BOOL operator != (const Iterator &other) const
        {
            return ((m_tree != other.m_tree) || (m_node != other.m_node));
        }

        // operator * - Dereference operator for Set Iterators.
        //
        //  Note:  The reference returned by this function is "const", so the
        //    value referenced by the Iterator may not be modified through the
        //    Iterator. This is a departure from STL iterator behavior.
        //
        //    Also, dereferencing an Iterator which does not reference a valid
        //    value in the Set is undefined and will almost certainly cause a
        //    crash.
        //
        //  Return Value:
        //
        //    Returns a const reference to the key in the Map referenced by the
        //    Iterator.
        //
        const Tk& operator * () const
        {
            return m_node->key;
        }

        // operator ++ - Prefix increment operator for Set Iterators. Causes the
        //   Iterator to reference the in-oder successor of the key currently
        //   referenced by the Iterator. If the Iterator is currently
        //   referencing the largest key in the Map, then the resulting Iterator
        //   will reference the Set's end (the NULL pair).
        //
        //  Note: Incrementing an Iterator which does not reference a valid
        //    key in the Set is undefined and will almost certainly cause a
        //    crash.
        //
        //  Return Value:
        //
        //    Returns the Iterator after it has been incremented.
        // 
        Iterator& operator ++ (int)
        {
            m_node = m_tree->next(m_node);
            return *this;
        }

        // operator ++ - Postfix increment operator for Map Iterators. Causes
        //   the Iterator to reference the in-order successor of the key/value
        //   pair currently referenced by the Iterator. If the Iterator is
        //   currently referencing the largest key/value pair in the Map, then
        //   the resulting Iterator will reference the Map's end (the NULL
        //   pair).
        //
        //  Note: Incrementing an Iterator which does not reference a valid
        //    key/value pair in the Map is undefined and will almost certainly
        //    cause a crash.
        //
        //  Return Value:
        //
        //    Returns the Iterator before it has been incremented.
        // 
        Iterator operator ++ ()
        {
            Tree<Tk>::node_t *cur = m_node;

            m_node = m_tree->next(m_node);
            return Iterator(m_tree, cur);
        }

        // operator - - Subtraction operator for Set Iterators. Causes the
        //   the Iterator to reference a key that is an in-order predecessor of
        //   the currently refereced key.
        //
        //  - num (IN): Number indicating the number of preceding keys to
        //      decrement the iterator.
        //
        //  Return Value:
        //
        //    Returns an Iterator referencing the key that precedes the original
        //    Iterator by "num" keys.
        //
        Iterator operator - (SIZE_T num) const
        {
            SIZE_T            count;
            Tree<Tk>::node_t *cur = m_node;

            cur = m_tree->prev(m_node);
            for (count = 0; count < num; count++)  {
                cur = m_tree->prev(m_node);
                if (cur == NULL) {
                    return Iterator(m_tree, NULL);
                }
            }
            return Iterator(m_tree, cur);
        }

        // operator == - Equality operator for Set Iterators. Set Iterators are
        //   considered equal if and only if they both refernce the same
        //   key in the same Set.
        //
        //  - other (IN): The other Set Iterator to compare against.
        //
        //  Return Value:
        //
        //    Returns true if the specified Set Iterator is equal to this Set
        //    Iterator; otherwise returns false.
        //
        BOOL operator == (const Iterator &other) const
        {
            return ((m_tree == other.m_tree) && (m_node == other.m_node));
        }

    private:
        // Private constructor. Only the Set class itself may use this
        //   constructor. It is used for constructing Iterators which reference
        //   specific nodes in the internal tree's structure.
        Iterator (const Tree<Tk> *tree, Tree<Tk>::node_t *node)
        {
            m_node = node;
            m_tree = tree;
        }

        Tree<Tk>::node_t *m_node; // Pointer to the node referenced by the Set Iterator.
        const Tree<Tk>   *m_tree; // Pointer to the tree containing the referenced node.

        // The Set class is a friend of Set Iterators.
        friend class Set<Tk>;
    };

    // begin - Obtains an Iterator referencing the beginning of the Set (i.e.
    //   the lowest key currently stored in the Set).
    //
    //  Return Value:
    //
    //    Returns an Iterator referencing the first key in the Set. If no keys
    //    are currenly stored in the Set, returns the "NULL" Iterator.
    //
    Iterator begin () const
    {
        return Iterator(&m_tree, m_tree.begin());
    }

    // clear - Erases all keys from the Set.
    //
    //   Note: This function does not, indeed cannot, free any dynamically
    //     allocated keys. Dynamically allocated keys must be manually
    //     deleted. Also, this function doesn't actually release any memory
    //     used by the Set, instead the memory remains reserved for future use
    //     by the Set.
    //
    //  Return Value:
    //
    //    None.
    //
    VOID clear ()
    {
        m_tree.clear();
    }

    // end - Obtains an Iterator referencing the end of the Set. The end of
    //   the Set does not reference an actual key. Instead it represents a
    //   "null" key which signifies the end (i.e. just beyond largest key
    //   currently stored in the Set). Also known as the "NULL" Iterator.
    Iterator end () const
    {
        return Iterator(&m_tree, NULL);
    }

    // erase - Erases a key from the Set.
    //
    //  - it (IN): Iterator referencing the key to be erased from the Set.
    //
    //  Return Value:
    //
    //    None.
    //
    VOID erase (Iterator& it)
    {
        m_tree.erase(it.m_node);
    }

    // erase - Erases a key from the Set.
    //
    //  - key (IN): The key to be erased from the Set.
    //
    //  Return Value:
    //
    //    None.
    //
    VOID erase (const Tk &key)
    {
        m_tree.erase(key);
    }

    // find - Finds a key in the Set.
    //
    //  - key (IN): The key to be found.
    //
    //  Return Value:
    //
    //    Returns an Iterator referencing the found key. If the key could not
    //    be found, then the "NULL" Iterator is returned.
    //
    Iterator find (const Tk &key) const
    {
        return Iterator(&m_tree, m_tree.find(key));
    }

    // insert - Inserts a key into the Set.
    //
    //  - key (IN): The key to be inserted.
    //
    //  Return Value:
    //
    //    Returns an Iterator referencing the key after it has been inserted
    //    into the Set.
    //
    Iterator insert (const Tk &key)
    {
        return Iterator(&m_tree, m_tree.insert(key));
    }

    // reserve - Sets the reserve size of the Set. The reserve size is the
    //   number of keys for which space should be pre-allocated to avoid
    //   frequent heap hits when inserting new keys into the Set.
    //
    //  - count (IN): The number of keys for which to reserve space in advance.
    //
    //  Return Value:
    //
    //    Returns the reserve size previously in use by the Set.
    //
    size_t reserve (size_t count)
    {
        return m_tree.reserve(count);
    }

private:
    // Private data
    Tree<Tk> m_tree; // The keys are actually stored in a tree.
};