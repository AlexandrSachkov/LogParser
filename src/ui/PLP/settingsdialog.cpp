/*
 * This file is part of the Line Catcher distribution (https://github.com/AlexandrSachkov/LineCatcher).
 * Copyright (c) 2019 Alexandr Sachkov.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "settingsdialog.h"
#include "mainwindow.h"

#include "Utils.h"

#include <QBoxLayout>
#include <QPushButton>
#include <QFormLayout>
#include <QSlider>
#include <QSettings>

SettingsDialog::SettingsDialog(
        PLP::CoreI* plpCore,
        const std::function<void(int)>& changeViewerFont,
        QWidget* parent) : QWidget(parent)
{
    _plpCore = plpCore;

    QFormLayout* mainLayout = new QFormLayout();
    this->setLayout(mainLayout);
    mainLayout->setContentsMargins(5, 5, 5, 5);
    mainLayout->setSpacing(10);

    setWindowFlags(Qt::Dialog | Qt::WindowCloseButtonHint);
    setWindowTitle("Settings");
    setWindowModality(Qt::WindowModal);
    setMinimumWidth(400);
    setMinimumHeight(300);

    _viewerFontSize = new QSlider(Qt::Orientation::Horizontal, this);
    mainLayout->addRow("Viewer font size:", _viewerFontSize);
    _viewerFontSize->setTickPosition(QSlider::TickPosition::TicksBelow);
    _viewerFontSize->setTickInterval(1);
    _viewerFontSize->setMinimum(10);
    _viewerFontSize->setMaximum(24);
    _viewerFontSize->setToolTip("Changes font size for file, index and script viewers");
    connect(_viewerFontSize, &QSlider::sliderReleased, [&, changeViewerFont](){
        changeViewerFont(_viewerFontSize->value());
    });

    QSettings settings("AlexandrSachkov", "LC");
    settings.beginGroup("Settings");

    _deleteMetaFiles = new QCheckBox(this);
    _deleteMetaFiles->setToolTip("Deletes file indexes (" +
         QString::fromUtf8(PLP::FILE_RANDOM_ACCESS_INDEX_EXTENSION) +
         ") and search results (" +
         QString::fromUtf8(PLP::FILE_INDEX_EXTENSION) +
         ") when application is closed");

    connect(_deleteMetaFiles, &QCheckBox::stateChanged, [&](){
        _plpCore->cleanupGeneratedFilesOnRelease(_deleteMetaFiles->isChecked());
    });
    _deleteMetaFiles->setChecked(settings.value("cleanupFiles", false).toBool());
    mainLayout->addRow("Cleanup files on closure:", _deleteMetaFiles);

    QFont font = this->font();
    font.setPointSize(12);
    this->setFont(font);

    int viewerFontSizeVal = settings.value("viewerFontSize", 14).toInt();
    QMetaObject::invokeMethod(_viewerFontSize, [&, viewerFontSizeVal](){
        _viewerFontSize->setValue(viewerFontSizeVal);
        changeViewerFont(viewerFontSizeVal);
    });

    settings.endGroup();
}

void SettingsDialog::hideEvent(QHideEvent* event) {
    QSettings settings("AlexandrSachkov", "LC");
    settings.beginGroup("Settings");
    settings.setValue("viewerFontSize", _viewerFontSize->value());
    settings.setValue("cleanupFiles", _deleteMetaFiles->isChecked());
    settings.endGroup();
}
