// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_UI_QT_MODEL_HPP
#define OPENTXS_UI_QT_MODEL_HPP

#include <QAbstractItemModel>
#include <QHash>
#include <QMetaType>
#include <QObject>
#include <QString>
#include <QVariant>
#include <memory>

class QByteArray;
class QObject;

namespace opentxs
{
namespace ui
{
namespace internal
{
struct Row;
}  // namespace internal

namespace qt
{
namespace internal
{
struct Index;
struct Model;
}  // namespace internal

class Model;
class ModelHelper;
class RowWrapper;
}  // namespace qt
}  // namespace ui
}  // namespace opentxs

class OPENTXS_EXPORT opentxs::ui::qt::RowWrapper final : public QObject
{
    Q_OBJECT
public:
    std::shared_ptr<opentxs::ui::internal::Row> row_;

    RowWrapper() noexcept;
    RowWrapper(std::shared_ptr<opentxs::ui::internal::Row>) noexcept;
    RowWrapper(const RowWrapper&) noexcept;
    auto operator=(const RowWrapper&) noexcept -> RowWrapper&;

    ~RowWrapper() final;

private:
    RowWrapper(RowWrapper&&) = delete;
    auto operator=(RowWrapper&&) noexcept -> RowWrapper& = delete;
};

Q_DECLARE_OPAQUE_POINTER(opentxs::ui::internal::Row*)
Q_DECLARE_METATYPE(opentxs::ui::qt::RowWrapper)
Q_DECLARE_METATYPE(opentxs::ui::internal::Row*)

namespace opentxs::ui::qt
{
class ModelHelper final : public QObject
{
    Q_OBJECT

signals:
    void changeRow(ui::internal::Row* parent, ui::internal::Row* row);
    void deleteRow(ui::internal::Row* row);
    void insertRow(
        ui::internal::Row* parent,
        ui::internal::Row* after,
        opentxs::ui::qt::RowWrapper row);
    void moveRow(
        ui::internal::Row* newParent,
        ui::internal::Row* newBefore,
        ui::internal::Row* row);

public slots:
    void requestChangeRow(
        ui::internal::Row* parent,
        ui::internal::Row* row) noexcept;
    void requestDeleteRow(ui::internal::Row* row) noexcept;
    void requestInsertRow(
        ui::internal::Row* parent,
        ui::internal::Row* after,
        std::shared_ptr<ui::internal::Row> row) noexcept;
    void requestMoveRow(
        ui::internal::Row* newParent,
        ui::internal::Row* newBefore,
        ui::internal::Row* row) noexcept;

public:
    ModelHelper(Model* model) noexcept;

    ~ModelHelper() final;

private:
    ModelHelper() = delete;
    ModelHelper(const ModelHelper&) = delete;
    ModelHelper(ModelHelper&&) = delete;
    ModelHelper& operator=(const ModelHelper&) = delete;
    ModelHelper& operator=(ModelHelper&&) = delete;
};

class OPENTXS_EXPORT Model : public QAbstractItemModel
{
    Q_OBJECT

public:
    auto columnCount(const QModelIndex& parent = {}) const noexcept
        -> int final;
    auto data(const QModelIndex& index, int role = Qt::DisplayRole)
        const noexcept -> QVariant final;
    auto hasChildren(const QModelIndex& parent = {}) const noexcept
        -> bool final;
    auto headerData(
        int section,
        Qt::Orientation orientation,
        int role = Qt::DisplayRole) const noexcept -> QVariant override;
    auto index(int row, int column, const QModelIndex& parent = {})
        const noexcept -> QModelIndex final;
    auto parent(const QModelIndex& index) const noexcept -> QModelIndex final;
    auto roleNames() const noexcept -> QHash<int, QByteArray> final;
    auto rowCount(const QModelIndex& parent = {}) const noexcept -> int final;

    ~Model() override;

protected:
    internal::Model* internal_;

    Model(internal::Model* internal) noexcept;

private slots:
    void changeRow(ui::internal::Row* parent, ui::internal::Row* row) noexcept;
    void deleteRow(ui::internal::Row* row) noexcept;
    void insertRow(
        ui::internal::Row* parent,
        ui::internal::Row* after,
        opentxs::ui::qt::RowWrapper row) noexcept;
    void moveRow(
        ui::internal::Row* newParent,
        ui::internal::Row* newBefore,
        ui::internal::Row* row) noexcept;

private:
    friend ModelHelper;

    auto make_index(const internal::Index& index) const noexcept -> QModelIndex;

    Model() = delete;
    Model(const Model&) = delete;
    Model(Model&&) = delete;
    Model& operator=(const Model&) = delete;
    Model& operator=(Model&&) = delete;
};
}  // namespace opentxs::ui::qt
#endif
