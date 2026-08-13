// SPDX-License-Identifier: GPL-3.0-only
/*
 *  Prism Launcher - Minecraft Launcher
 *  Copyright (C) 2022 Sefa Eyeoglu <contact@scrumplex.net>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, version 3.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * This file incorporates work covered by the following copyright and
 * permission notice:
 *
 *      Copyright 2013-2021 MultiMC Contributors
 *
 *      Licensed under the Apache License, Version 2.0 (the "License");
 *      you may not use this file except in compliance with the License.
 *      You may obtain a copy of the License at
 *
 *          http://www.apache.org/licenses/LICENSE-2.0
 *
 *      Unless required by applicable law or agreed to in writing, software
 *      distributed under the License is distributed on an "AS IS" BASIS,
 *      WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *      See the License for the specific language governing permissions and
 *      limitations under the License.
 */

#include "MinecraftAccount.h"

#include <QDir>
#include <QIcon>
#include <QUuid>
#include <QDebug>

#include "Application.h"
#include "FileSystem.h"

#include "tasks/Task.h"

MinecraftAccount::MinecraftAccount(QObject* parent) : QObject(parent)
{
    m_internalId = QUuid::createUuid().toString();
}

MinecraftAccountPtr MinecraftAccount::createOffline(const QString& username)
{
    MinecraftAccountPtr account(new MinecraftAccount());
    account->m_type = AccountType::Offline;
    account->m_profileName = username;
    account->m_profileId = "OfflinePlayer:" + username;
    account->m_yggdrasilToken = "OfflineToken";
    account->m_accessToken = "OfflineToken";
    account->m_validity = AccountState::Online;
    return account;
}

MinecraftAccountPtr MinecraftAccount::loadFromJsonV3(const QJsonObject& json)
{
    MinecraftAccountPtr account(new MinecraftAccount());
    if (account->loadFromJsonV3Internal(json)) {
        return account;
    }
    return nullptr;
}

bool MinecraftAccount::shouldRefresh() const
{
    if (m_type == AccountType::Offline) {
        return false;
    }
    return m_validity != AccountState::Online;
}

QPixmap MinecraftAccount::getFace(int width, int height) const
{
    if (!m_face.isNull()) {
        return m_face.scaled(width, height, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }
    return QPixmap();
}

shared_qobject_ptr<AuthFlow> MinecraftAccount::refresh()
{
    if (m_type == AccountType::Offline) {
        m_validity = AccountState::Online;
        emit changed();
        return nullptr;
    }
    return m_currentTask;
}

shared_qobject_ptr<AuthFlow> MinecraftAccount::login()
{
    return m_currentTask;
}

shared_qobject_ptr<AuthFlow> MinecraftAccount::currentTask()
{
    return m_currentTask;
}

void MinecraftAccount::authSucceeded()
{
    m_validity = AccountState::Online;
    emit changed();
}

void MinecraftAccount::authFailed(const QString& reason)
{
    qWarning() << "Account authentication failed:" << reason;
    m_validity = AccountState::Errored;
    emit changed();
}
