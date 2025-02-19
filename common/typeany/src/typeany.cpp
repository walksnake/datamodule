/**
 * @file typeany.cpp
 * @brief
 * @author Fan Chuanlin, fanchuanlin@aliyun.com
 * @version 1.0
 * @date 2020-10-05
 * @attention
 * @brief History:
 * <table>
 * <tr><th> Date <th> Version <th> Author <th> Description
 * <tr><td> xxx-xx-xx <td> 1.0 <td> Fan Chuanlin <td> Create
 * </table>
 *
 */
#include "../inc/typeany.h"

/// TypeAny
TypeAny::TypeAny( TypeAny &in_val )
{

}

/// Copy
void TypeAny::AllCopy( const TypeAny& in_val )
{

}

/**
 * @brief  insert data to v.xxx
 *
 * @param[in]  teid  typebase + type extend
 * @param[in]  pstream source data pointer
 *
 * @returns
 */
BOOLEAN TypeAny::Insert( TYPEANY_ID teid, const void * pstream )
{
    return TRUE;
}

/// get value as param in
const void * TypeAny::ParamIn() const
{
    if( m_typeany_id == TYPE_BASE_NULL )
    {
        return NULL;

    }
    else if( m_typeany_id == TYPE_BASE_OCTET )
    {
        return &(v.m_octet);
    }
    else if( m_typeany_id == TYPE_BASE_CHAR )
    {
        return &(v.m_char);
    }
    else if( m_typeany_id == TYPE_BASE_SHORT )
    {
        return &(v.m_short);
    }
    else if( m_typeany_id == TYPE_BASE_USHORT )
    {
        return &(v.m_ushort);
    }
    else if( m_typeany_id == TYPE_BASE_INT )
    {
        return &(v.m_int);
    }
    else if( m_typeany_id == TYPE_BASE_UINT )
    {
        return &(v.m_uint);
    }
    else if( m_typeany_id == TYPE_BASE_INT64 )
    {
        return &(v.m_int64);
    }
    else if( m_typeany_id == TYPE_BASE_UINT64 )
    {
        return &(v.m_int64);
    }
    else if( m_typeany_id == TYPE_BASE_FLOAT )
    {
        return &(v.m_float);
    }
    else if( m_typeany_id == TYPE_BASE_DOUBLE )
    {
        return &(v.m_double);
    }
    else if( m_typeany_id == TYPE_BASE_BOOLEAN )
    {
        return &(v.m_boolean);
    }
    else if( m_typeany_id == TYPE_BASE_BUFF_PTR )
    {
        return v.m_buff_ptr;
    }
    else
    {
        return v.m_void;
    }
}

/// get value as param out
void* TypeAny::ParamOut()
{
    return v.m_buff_ptr;
}

/// get value as param in/out
void* TypeAny::ParamInOut()
{
    return v.m_buff_ptr;
}


/**
 * @brief  check the type base
 *
 * @param[in]  tid
 *
 * @returns
 */
BOOLEAN TypeAny::IsSimpleDateType( TYPEANY_ID tid )
{
    TYPEANY_ID tmpid = tid  >> TYPE_BASE_SHIFTBITS;
    if( tmpid >= TYPE_BASE_OCTET && tmpid <= TYPE_BASE_BOOLEAN )
    {
        return TRUE;
    }
    return FALSE;
}

/// free memory
void TypeAny::AllFree()
{
    if( IsSimpleDateType( m_typeany_id ) )
    {
        LOG( INFO ) << "Simple Type free!" << "TID " << m_typeany_id;
    }
    else if( m_typeany_id == TYPE_BASE_BUFF_PTR )
    {
        delete v.m_buff_ptr;
        v.m_buff_ptr = NULL;
    }
    else if( m_typeany_id == TYPE_BASE_STR_PTR )
    {
        delete v.m_string_ptr;
        v.m_string_ptr = NULL;
    }
    else
    {
        LOG( ERROR ) << "Error Type free!" << "TID " << m_typeany_id;
    }

    m_typeany_id = 0;
}

