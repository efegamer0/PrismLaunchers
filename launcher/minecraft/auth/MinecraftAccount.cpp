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
 */

#include "MinecraftAccount.h"
#include "AccountData.h"
#include "Parsers.h"

#include <QUuid>
#include <QDebug>

MinecraftAccount::MinecraftAccount(QObject* parent) : QObject(parent)
{
    m_accountData.internalId = QUuid::createUuid().toString();
}

MinecraftAccountPtr MinecraftAccount::createOffline(const QString& username)
{
    MinecraftAccountPtr account(new MinecraftAccount());
    account->m_accountData.type = AccountType::Offline;
    account->m_accountData.profileName = username;
    account->m_accountData.profileId = "OfflinePlayer:" + username;
    account->m_accountData.yggdrasilToken = "OfflineToken";
    account->m_accountData.accessToken = "OfflineToken";
    account->m_accountData.validity = AccountState::Online;
    return account;
}

MinecraftAccountPtr MinecraftAccount::loadFromJsonV3(const QJsonObject& json)
{
    MinecraftAccountPtr account(new MinecraftAccount());
    if (Parsers::parseAccountData(json, account->m_accountData)) {
        return account;
    }
    return nullptr;
}

QJsonObject MinecraftAccount::saveToJson() const
{
    return Parsers::serializeAccountData(m_accountData);
}

bool MinecraftAccount::shouldRefresh() const
{
    if (m_accountData.type == AccountType::Offline) {
        return false;
    }
    return m_accountData.validity != AccountState::Online;
}

QPixmap MinecraftAccount::getFace(int width, int height) const
{
    if (!m_accountData.face.isNull()) {
        return m_accountData.face.scaled(width, height, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }
    return QPixmap();
}

shared_qobject_ptr<AuthFlow> MinecraftAccount::refresh()
{
    if (m_accountData.type == AccountType::Offline) {
        m_accountData.validity = AccountState::Online;
        emit changed();
        return nullptr;
    }
    return m_currentTask;
}

shared_qobject_ptr<AuthFlow> MinecraftAccount::currentTask()
{
    return m_currentTask;
}
