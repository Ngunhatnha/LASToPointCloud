#ifndef _Array_h__
#define _Array_h__
#include <limits.h>
    /**
     * A simpler, compacter alternative to std::vector.<br/><br/>
     *
     * Length is explicit - there is no hidden reserve.<br/><br/>
     *
     * @invariants
     * * pStorage_m is 0 or a valid address<br/>
     * * length_m is >= 0 and <= getMaxLength() (INT_MAX)<br/>
     */
    template<class TYPE>
    class Array
    {
        /// standard object services ---------------------------------------------------
    public:
        Array();
        explicit       Array(int length);                               // throws

        ~Array();
        Array(const Array&);                               // throws
        Array& operator=(const Array&);                           // throws


/// commands -------------------------------------------------------------------
        void   setLength(int length);                           // throws

        void   swap(Array&);
        void   append(const TYPE&);                               // throws
        void   remove(int index);                                 // throws

        void   zeroStorage();

        TYPE* getStorage();
        TYPE& operator[](int index);


        /// queries --------------------------------------------------------------------
        int  getLength()                                             const;
        bool   isEmpty()                                               const;
        static  int  getMaxLength();

        const TYPE* getStorage()                                      const;
        const TYPE& operator[](int index)                           const;


        /// implementation -------------------------------------------------------------
    protected:
        void   assign(const Array<TYPE>&);

        void   acquireStorage(int length,
            bool  isCopied);

        static  void   copyObjects(TYPE* lValStart,
            const TYPE* rValStart,
            int       length);


        /// fields ---------------------------------------------------------------------
    private:
        TYPE* pStorage_m;
        int length_m;
    };

    /// standard object services ---------------------------------------------------
    template<class TYPE>
    Array<TYPE>::Array()
        : pStorage_m(0)
        , length_m(0)
    {
    }


    template<class TYPE>
    Array<TYPE>::Array
    (
        const int length
    )
        : pStorage_m(0)
        , length_m(0)
    {
        Array<TYPE>::setLength(length);
    }


    template<class TYPE>
    Array<TYPE>::~Array()
    {
        delete[] pStorage_m;
    }


    template<class TYPE>
    Array<TYPE>::Array
    (
        const Array<TYPE>& other
    )
        : pStorage_m(0)
        , length_m(0)
    {
        Array<TYPE>::assign(other);
    }


    template<class TYPE>
    Array<TYPE>& Array<TYPE>::operator=
        (
            const Array<TYPE>& other
            )
    {
        assign(other);

        return *this;
    }

    /// commands -------------------------------------------------------------------
    template<class TYPE>
    void Array<TYPE>::setLength
    (
        const int length
    )
    {
        acquireStorage(length, false);
    }


    template<class TYPE>
    void Array<TYPE>::swap
    (
        Array<TYPE>& other
    )
    {
        TYPE* const tmpM = pStorage_m;
        pStorage_m = other.pStorage_m;
        other.pStorage_m = tmpM;

        const int tmpL = length_m;
        length_m = other.length_m;
        other.length_m = tmpL;
    }


    template<class TYPE>
    void Array<TYPE>::append
    (
        const TYPE& element
    )
    {
        // expand storage, duplicating elements
        acquireStorage(length_m + 1, true);

        // write new element into last position
        *(pStorage_m + length_m - 1) = element;


        // // make larger storage
        // Array<TYPE> newArray( length_m + 1 );
        //
        // // duplicate elements, and append new element
        // copyObjects( newArray.pStorage_m, pStorage_m, length_m );
        // *(newArray.pStorage_m + length_m) = element;
        //
        // // swap new storage with this
        // swap( newArray );
    }


    template<class TYPE>
    void Array<TYPE>::remove
    (
        const int index
    )
    {
        // check index is within range
        if ((index >= 0) & (index < length_m))
        {
            // make smaller storage
            Array<TYPE> newArray(length_m - 1);

            // copy elements, skipping element at index
            {
                TYPE* pDestination = newArray.pStorage_m;
                TYPE* pEnd = pDestination + newArray.length_m;
                const TYPE* pSource = pStorage_m;
                const TYPE* pIndex = pSource + index;
                while (pDestination < pEnd)
                {
                    pSource += static_cast<int>(pSource == pIndex);
                    *(pDestination++) = *(pSource++);
                }
            }

            // swap new storage with this
            swap(newArray);
        }
    }


    template<class TYPE>
    void Array<TYPE>::zeroStorage()
    {
        for (int i = length_m; i-- > 0; )
        {
            pStorage_m[i] = TYPE();
        }
    }


    template<class TYPE>
    inline
        TYPE* Array<TYPE>::getStorage()
    {
        return pStorage_m;
    }


    template<class TYPE>
    inline
        TYPE& Array<TYPE>::operator[]
        (
            const int index
            )
    {
        return pStorage_m[index];
    }




    /// queries --------------------------------------------------------------------
    template<class TYPE>
    inline
        int Array<TYPE>::getLength() const
    {
        return length_m;
    }


    template<class TYPE>
    inline
        bool Array<TYPE>::isEmpty() const
    {
        return 0 == length_m;
    }


    template<class TYPE>
    inline
        int Array<TYPE>::getMaxLength()
    {
        return INT_MAX;
    }


    template<class TYPE>
    inline
        const TYPE* Array<TYPE>::getStorage() const
    {
        return pStorage_m;
    }


    template<class TYPE>
    inline
        const TYPE& Array<TYPE>::operator[]
        (
            const int index
            ) const
    {
        return pStorage_m[index];
    }




    /// implementation -------------------------------------------------------------
    template<class TYPE>
    void Array<TYPE>::assign
    (
        const Array<TYPE>& other
    )
    {
        if (&other != this)
        {
            acquireStorage(other.getLength(), false);

            copyObjects(getStorage(), other.getStorage(), other.getLength());
        }
    }


    template<class TYPE>
    void Array<TYPE>::acquireStorage
    (
        int      newLength,
        const bool isCopied
    )
    {
        // clamp to 0 min
        newLength = (newLength >= 0) ? newLength : 0;

        // only allocate if different length
        if (newLength != length_m)
        {
            // allocate new storage
            TYPE* pNewStorage = new TYPE[newLength];

            // copy elements to new storage
            if (isCopied)
            {
                copyObjects(pNewStorage, pStorage_m,
                    (length_m <= newLength ? length_m : newLength));
            }

            // delete old storage and set the members
            delete[] pStorage_m;
            pStorage_m = pNewStorage;
            length_m = newLength;
        }
    }


    template<class TYPE>
    void Array<TYPE>::copyObjects
    (
        TYPE* const       pDestination,
        const TYPE* const pSource,
        const int      length
    )
    {
        if (length >= 0)
        {
            TYPE* pDestinationCursor = pDestination + length;
            const TYPE* pSourceCursor = pSource + length;

            while (pDestinationCursor > pDestination)
            {
                *(--pDestinationCursor) = *(--pSourceCursor);
            }
        }
    }
 
#endif//Array_h