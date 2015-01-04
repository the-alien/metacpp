#include "SqlResultIterator.h"
#include "SqlResultSet.h"
#include "SqlTransaction.h"

namespace metacpp
{
namespace sql
{

SqlResultIterator::SqlResultIterator(SqlResultSetData *resultSet, int rowId)
    : m_resultSet(resultSet), m_rowId(rowId)
{
}

SqlResultIterator::SqlResultIterator(const SqlResultIterator &other)
{
    *this = other;
}

SqlResultIterator::~SqlResultIterator()
{
}

SqlResultIterator &SqlResultIterator::operator=(const SqlResultIterator &rhs)
{
    m_resultSet = rhs.m_resultSet;
    m_rowId = rhs.m_rowId;
}

bool SqlResultIterator::operator ==(const SqlResultIterator& rhs)
{
    return m_resultSet == rhs.m_resultSet && m_rowId == rhs.m_rowId;
}

bool SqlResultIterator::operator !=(const SqlResultIterator& rhs)
{
    return !(*this == rhs);
}

const Object *SqlResultIterator::operator*() const
{
    return m_resultSet->m_storable->record();
}

const Object *SqlResultIterator::operator->() const
{
    return m_resultSet->m_storable->record();
}

SqlResultIterator& SqlResultIterator::operator ++()
{
    if (fetchNext())
        m_rowId++;
    else
        m_rowId = ROW_ID_PAST_THE_END;
    return *this;
}

bool SqlResultIterator::fetchNext()
{
    return m_resultSet->m_transaction.impl()->fetchNext(m_resultSet->m_statement->impl(),
                                                        m_resultSet->m_storable);
}

} // namespace sql
} // namespace metacpp
