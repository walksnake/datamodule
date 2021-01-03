/**
 * @file typeany.h
 * @brief
 * @author Fan Chuanlin, fanchuanlin@aliyun.com
 * @version 1.0
 * @date 2020-10-05
 * @attention
 * @brief History:
 * <table>
 * <tr><th> Date <th> Version <th> Author <th> Description
 * <tr><td> 2020-10-05 <td> 1.0 <td> Fan Chuanlin <td> Create
 * </table>
 *
 */

#ifndef _TYPEANY_H_
#define _TYPEANY_H_

#include "../../../include/normal_typedef.h"
#include "../../../feature/log/inc/easylogging.h"


using namespace std;

typedef class TypeAny Any;


/// enum for type base define
typedef enum
{
    TYPE_BASE_NULL = 0,
    TYPE_BASE_OCTET = 1,
    TYPE_BASE_CHAR = 2,
    TYPE_BASE_SHORT = 3,
    TYPE_BASE_USHORT = 4,
    TYPE_BASE_INT = 5,
    TYPE_BASE_UINT = 6,
    TYPE_BASE_INT64 = 7,
    TYPE_BASE_UINT64 = 8,
    TYPE_BASE_FLOAT = 9,
    TYPE_BASE_DOUBLE = 10,
    TYPE_BASE_BOOLEAN = 11,
    TYPE_BASE_BUFF_PTR = 12,
    TYPE_BASE_STR_PTR = 13,
    TYPE_BASE_VOID = 14,
} TYPE_BASE_E;


/// string for type description
const char type_base_str [][30] =
{
    "TYPE_BASE_NULL",
    "TYPE_BASE_OCTET",
    "TYPE_BASE_CHAR",
    "TYPE_BASE_SHORT",
    "TYPE_BASE_USHORT",
    "TYPE_BASE_INT",
    "TYPE_BASE_UINT",
    "TYPE_BASE_INT64",
    "TYPE_BASE_UINT64",
    "TYPE_BASE_FLOAT",
    "TYPE_BASE_DOUBLE",
    "TYPE_BASE_BOOLEAN",
    "TYPE_BASE_BUFF_PTR",
    "TYPE_BASE_STR_PTR",
    "TYPE_BASE_VOID",
};

/// enum for type extern define
typedef enum
{
    TYPE_EXTEND_ATOM = 0,
    TYPE_EXTEND_POINTER = 1,
    TYPE_EXTEND_SEQUENCE = 2,
    TYPE_EXTEND_ENUM1 = 3,
    TYPE_EXTEND_ENUM2 = 4,
    TYPE_EXTEND_ENUMR3 = 5,
    TYPE_EXTEND_ENUMR4 = 6,
    TYPE_EXTEND_TABLE = 7,
} TYPE_EXTEND_E;


const char type_extend_str [][30] =
{
    "TYPE_EXTEND_ATOM",
    "TYPE_EXTEND_POINTER",
    "TYPE_EXTEND_SEQUENCE",
    "TYPE_EXTEND_ENUM1",
    "TYPE_EXTEND_ENUM2",
    "TYPE_EXTEND_ENUM3",
    "TYPE_EXTEND_ENUM4",
    "TYPE_EXTEND_TABLE",
};

/**
 * @name data type define:
 * |-------------24bits-------------|------8bits-----|
 * |-------------type base----------|--type extend---|
 *
 * @{ */

#define TYPE_BASE_SHIFTBITS 8
#define TYPE_EXTEND_MASK 0xff

#define TID_TO_TEID(tid)    ((TYPEANY_ID)(tid<<TYPE_BASE_SHIFTBITS))
#define TID_TO_TEID2(tid, teid) ((TYPEANY_ID)(tid<<TYPE_BASE_SHIFTBITS)|teid)
#define TEID_TO_TID(teid) ((TYPEANY_ID)(teid >> TYPE_BASE_SHIFTBITS))
#define TEID_TO_TYPE(teid) ((TYPEANY_ID)(teid & TYPE_EXTEND_MASK))

/**  @} */

/**
 * @brief  we can use void * to process any type, but we need know the real type to transfer it,
 * so there is m_typeid to tell you how to parse the void pointer.
 */
class TypeAny
{
public:
    TYPEANY_ID m_typeany_id;

    /// TypeAny  using a union struct store data
    union
    {
        UINT8 m_octet;
        INT8 m_char;
        INT16 m_short;
        UINT16 m_ushort;
        INT32 m_int;
        UINT32 m_uint;
        INT64 m_int64;
        UINT64 m_uint64;
        FLOAT  m_float;
        DOUBLE m_double;
        BOOLEAN m_boolean;
        OCTET *m_buff_ptr;
        CHAR *m_string_ptr;
        VOID *m_void;
    } v;

public:
    /// special type
    TypeAny()
    {
        m_typeany_id = 0;
    }

    /// UINT8
    TypeAny( UINT8 in_val, TYPEANY_ID teid = TYPE_BASE_OCTET )
    {
        m_typeany_id = teid << TYPE_BASE_SHIFTBITS;
        v.m_octet = in_val;
    }

    /// INT8
    TypeAny( INT8 in_val, TYPEANY_ID teid = TYPE_BASE_CHAR )
    {
        m_typeany_id = teid << TYPE_BASE_SHIFTBITS;
        v.m_char = in_val;
    }

    /// INT16
    TypeAny( INT16 in_val, TYPEANY_ID teid = TYPE_BASE_SHORT )
    {
        m_typeany_id = teid << TYPE_BASE_SHIFTBITS;
        v.m_short = in_val;
    }

    /// UINT16
    TypeAny( UINT16 in_val, TYPEANY_ID teid = TYPE_BASE_USHORT )
    {
        m_typeany_id = teid << TYPE_BASE_SHIFTBITS;
        v.m_ushort = in_val;
    }

    /// INT32
    TypeAny( INT32 in_val, TYPEANY_ID teid = TYPE_BASE_INT )
    {
        m_typeany_id = teid << TYPE_BASE_SHIFTBITS;
        v.m_int = in_val;
    }

    /// UINT32
    TypeAny( UINT32 in_val, TYPEANY_ID teid = TYPE_BASE_UINT )
    {
        m_typeany_id = teid << TYPE_BASE_SHIFTBITS;
        v.m_uint = in_val;
    }

    /// INT64
    TypeAny( INT64 in_val, TYPEANY_ID teid = TYPE_BASE_INT64 )
    {
        m_typeany_id = teid << TYPE_BASE_SHIFTBITS;
        v.m_int64 = in_val;
    }

    /// UINT64
    TypeAny( UINT64 in_val, TYPEANY_ID teid = TYPE_BASE_UINT64 )
    {
        m_typeany_id = teid << TYPE_BASE_SHIFTBITS;
        v.m_uint64 = in_val;
    }

    /// float
    TypeAny( FLOAT in_val, TYPEANY_ID teid = TYPE_BASE_FLOAT )
    {
        m_typeany_id = teid << TYPE_BASE_SHIFTBITS;
        v.m_float = in_val;
    }

    /// double
    TypeAny( DOUBLE in_val, TYPEANY_ID teid = TYPE_BASE_DOUBLE )
    {
        m_typeany_id = teid << TYPE_BASE_SHIFTBITS;
        v.m_double = in_val;
    }

    /// boolean
    TypeAny( BOOLEAN in_val, TYPEANY_ID teid = TYPE_BASE_BOOLEAN )
    {
        m_typeany_id = teid << TYPE_BASE_SHIFTBITS;
        v.m_boolean = in_val;
    }

    TypeAny( OCTET* in_val, TYPEANY_ID teid = TYPE_BASE_BOOLEAN )
    {
        m_typeany_id = teid << TYPE_BASE_SHIFTBITS;
        v.m_boolean = in_val;
    }

		/// normal datatype
    BOOLEAN IsSimpleDateType( TYPEANY_ID tid );

		/// free memory
    void AllFree();
   	
		~TypeAny()
    {
        AllFree();
    }

    /// TypeAny
    TypeAny( TypeAny &in_val );

    /// Copy
    void AllCopy( const TypeAny& in_val );

    /// Insert
    BOOLEAN Insert( TYPEANY_ID teid, const void * pstream );

    /// get value as param in
    const void* ParamIn() const;

    /// get value as param out
    void* ParamOut();

    /// get value as param in/out
    void* ParamInOut();

public:
    /// operator define
    TypeAny & operator = ( const TypeAny& in_val )
    {
        if( this != &in_val )
        {
            AllCopy( in_val );
        }
        return *this;
    }
};

#if 0
template<typename T>
void VariantSetValueDelete( set<T> *pSetValue )
{
    if ( pSetValue )
    {
        delete pSetValue;
        pSetValue = nullptr;

    }

}

template<typename T>
void VariantSetValueCopy( VariantValue &toValue, const set<T> *pFromValue )
{
    if ( nullptr == pFromValue )
        return;

    toValue.m_pSetVal = new set<T>( *pFromValue );

}

template<typename T>
uint32_t VariantSetValueSize( set<T> *pSetValue )
{
    return pSetValue ? static_cast<uint32_t>( pSetValue->size() ) : 0;

}

template<typename T>
bool VariantSetValueAt( set<T> *pSetValue, uint32_t uIndex, T &subValue )
{
    if ( nullptr == pSetValue )
        return false;

    auto        it = pSetValue->begin();

    advance( it, uIndex );
    if ( it == pSetValue->end() )
        return false;

    subValue = *it;
    return true;

}

template<typename T>
bool VariantSetValueAdd( set<T> *pSetValue, T value )
{
    if ( nullptr == pSetValue )
        return false;

    auto        it = pSetValue->find( value );

    if ( it == pSetValue->end() )
        return false;

    pSetValue->insert( value );
    return true;

}

template<typename T>
bool VariantSetValueRemove( set<T> *pSetValue, T value )
{
    if ( nullptr == pSetValue )
        return false;

    auto        it = pSetValue->find( value );

    if ( it == pSetValue->end() )
        return false;

    pSetValue->erase( it );
    return true;

}

#endif

#endif

