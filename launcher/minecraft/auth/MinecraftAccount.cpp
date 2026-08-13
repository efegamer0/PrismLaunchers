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
#include "AccountData.h"

#include <QDir>
#include <QIcon>
#include <QUuid>
#include <QDebug>

#include "Application.h"
#include "FileSystem.h"

#include "tasks/Task.h"

MinecraftAccount::MinecraftAccount(QObject* parent) : QObject(parent), m_data(new AccountData)
{
    m_data->internalId = QUuid::createUuid().toString();
}

MinecraftAccountPtr MinecraftAccount::createOffline(const QString& username)
{
    MinecraftAccountPtr account(new MinecraftAccount());
    account->m_data->type = AccountType::Offline;
    account->m_data->profileName = username;
    account->m_data->profileId = "OfflinePlayer:" + username;
    account->m_data->yggdrasilToken = "OfflineToken";
    account->m_data->accessToken = "OfflineToken";
    account->m_data->validity = AccountState::Online;
    return account;
}

MinecraftAccountPtr MinecraftAccount::loadFromJsonV3(const QJsonObject& json)
{
    MinecraftAccountPtr account(new MinecraftAccount());
    if (account->m_data->loadFromJsonV3(json)) {
        return account;
    }
    return nullptr;
}

QJsonObject MinecraftAccount::saveToJson() const
{
    return m_data->saveToJson();
}

QString MinecraftAccount::internalId() const
{
    return m_data->internalId;
}

AccountType MinecraftAccount::accountType() const
{
    return m_data->type;
}

AccountState MinecraftAccount::accountState() const
{
    return m_data->validity;
}

QString MinecraftAccount::profileName() const
{
    return m_data->profileName;
}

QString MinecraftAccount::profileId() const
{
    return m_data->profileId;
}

QString MinecraftAccount::accessToken() const
{
    return m_data->accessToken;
}

bool MinecraftAccount::shouldRefresh() const
{
    if (m_data->type == AccountType::Offline) {
        return false;
    }
    return m_data->validity != AccountState::Online;
}

QPixmap MinecraftAccount::getFace(int width, int height) const
{
    if (!m_data->face.isNull()) {
        return m_data->face.scaled(width, height, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }
    return QPixmap();
}

shared_qobject_ptr<AuthFlow> MinecraftAccount::refresh()
{
    if (m_data->type == AccountType::Offline) {
        m_data->validity = AccountState::Online;
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
    m_data->validity = AccountState::Online;
    emit changed();
}

void MinecraftAccount::authFailed(const QString& reason)
{
    qWarning() << "Account authentication failed:" << reason;
    m_data->validity = AccountState::Errored;
    emit changed();
}
