/****************************************************************************
* Copyright 2014-2015 Trefilov Dmitrij                                      *
*                                                                           *
* Licensed under the Apache License, Version 2.0 (the "License");           *
* you may not use this file except in compliance with the License.          *
* You may obtain a copy of the License at                                   *
*                                                                           *
*    http://www.apache.org/licenses/LICENSE-2.0                             *
*                                                                           *
* Unless required by applicable law or agreed to in writing, software       *
* distributed under the License is distributed on an "AS IS" BASIS,         *
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  *
* See the License for the specific language governing permissions and       *
* limitations under the License.                                            *
****************************************************************************/
#ifndef SQLSTORABLE_H
#define SQLSTORABLE_H
#include "config.h"
#include <cstdint>
#include <memory>
#include "Object.h"
#include "SqlStatement.h"
#include "SqlColumnConstraint.h"

namespace metacpp
{
namespace db
{
namespace sql
{
    /** \brief Base class for all sql-persistible objects */
    class SqlStorable
    {
    protected:
        /** \brief Constructs a new instance of SqlStorable */
        SqlStorable();
    public:
        virtual ~SqlStorable();

        /** \brief Returns MetaFieldBase for the primary key specified using SqlConstraintPrimaryKey */
        virtual const MetaFieldBase *primaryKey() const = 0;
        /** \brief Returns pointer to the persisting Object */
        virtual Object *record() = 0;

        /** \brief Creates a select statement */
        SqlStatementSelect select();
        /** \brief Creates a delete statement */
        SqlStatementDelete remove();
        /** \brief Create an update statement */
        SqlStatementUpdate update();

        /** Insert record into the database using specified transaction */
        bool insertOne(SqlTransaction& transaction);
        /** Persist changes on the record by primary key using specified transaction */
        bool updateOne(SqlTransaction& transaction);
        /** Delete the record by primary key using specified transaction */
        bool removeOne(SqlTransaction& transaction);
    protected:
        static void createSchema(SqlTransaction& transaction, const MetaObject *metaObject,
                                 const Array<SqlConstraintBasePtr>& constraints);
    private:
        ExpressionNodeWhereClause whereId();
        static void createSchemaSqlite(SqlTransaction& transaction, const MetaObject *metaObject,
                                       const Array<SqlConstraintBasePtr>& constraints);
        static void createSchemaPostgreSQL(SqlTransaction &transaction, const MetaObject *metaObject,
                                    const Array<SqlConstraintBasePtr> &constraints);
        static void createSchemaMySql(SqlTransaction& transaction, const MetaObject *metaObject,
                                      const Array<SqlConstraintBasePtr>& constraints);
    };

    /** \brief Common wrapper template class for Object.
     *
     * This class should be defined first using DEFINE_STORABLE macro
    */
    template<typename TObj, typename = typename std::enable_if<std::is_base_of<Object, TObj>::value>::type>
    class Storable : public SqlStorable, public TObj
    {
    public:
        /** \brief Constructs a new instance of Storable */
        Storable() : m_pkey(nullptr) {
        }

        Storable(TObj& obj) : TObj(obj), m_pkey(nullptr) {
        }

        /** \brief Overriden from SqlStorable::primaryKey */
        const MetaFieldBase *primaryKey() const override {
            if (m_pkey) return m_pkey;
            for (size_t i = 0; i < ms_constraints.size(); ++i)
                if (ms_constraints[i]->type() == SqlConstraintTypePrimaryKey)
                    return m_pkey = ms_constraints[i]->metaField();
            return nullptr;
        }

        /** \brief Gets a constraint at position \arg i */
        static SqlConstraintBasePtr getConstraint(size_t i) {
            return ms_constraints[i];
        }

        /** \brief Returns total number of constraints defined for this type */
        static size_t numConstraints() {
            return ms_constraints.size();
        }

        /** \brief Executes schema creation sql code using specified transaction */
        static void createSchema(SqlTransaction& transaction)
        {
            SqlStorable::createSchema(transaction, TObj::staticMetaObject(), ms_constraints);
        }

        static Array<TObj> fetchAll(SqlTransaction& transaction) {
            Array<TObj> result;
            Storable<TObj> storable;
            auto set = storable.select().exec(transaction);
            size_t size = set.size();
            if (size != std::numeric_limits<size_t>::max())
                result.reserve(size);
            for (auto row : set) {
                (void)row;
                result.push_back(storable);
            }
            return result;
        }

        static Array<TObj> fetchAll(SqlTransaction& transaction,
                             const ExpressionNodeWhereClause& whereClause)
        {
            Array<TObj> result;
            Storable<TObj> storable;
            auto set = storable.select().where(whereClause).exec(transaction);
            size_t size = set.size();
            if (size != std::numeric_limits<size_t>::max())
                result.reserve(size);
            for (auto row : set) {
                (void)row;
                result.push_back(storable);
            }
            return result;
        }

        static void insertAll(SqlTransaction& transaction,
                              const Array<TObj> objects)
        {
            Storable<TObj> storable;
            SqlStatementInsert statement(&storable);
            statement.execPrepare(transaction);
            for (auto& obj : objects)
            {
                statement.execStep(transaction, &obj);
            }
        }

    private:

        /** \brief Overriden from SqlStorable::record */
        Object *record() override {
            return this;
        }
    private:
        mutable const MetaFieldBase *m_pkey;
        static const Array<SqlConstraintBasePtr> ms_constraints;
    };

/** \brief Instantiate template class Storable<TObj> with a list of constraints
 *
 * TODO: example
 * \see SqlConstraintBase
 */
#define DEFINE_STORABLE(TObj, ...) \
template<> const Array<SqlConstraintBasePtr> Storable<TObj>::ms_constraints = { __VA_ARGS__ };

} // namespace sql
} // namespace db
} // namespace metacpp
#endif // SQLSTORABLE_H
