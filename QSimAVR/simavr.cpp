/* *************************************************************************
 *  Copyright 2012 Jakob Gruber                                            *
 *                                                                         *
 *  This program is free software: you can redistribute it and/or modify   *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation, either version 2 of the License, or      *
 *  (at your option) any later version.                                    *
 *                                                                         *
 *  This program is distributed in the hope that it will be useful,        *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *  You should have received a copy of the GNU General Public License      *
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.  *
 ************************************************************************* */



#include "simavr.h"

#include <avr_ioport.h>
#include <QCoreApplication>
#include <sim_gdb.h>
#include <stdio.h>

#define GDB_PORT (1234)
#define FREQUENCY (16000000)
#define MMCU ("atmega1280")

SimAVR::SimAVR() : avr(NULL)
{
}

SimAVR::~SimAVR()
{
    if (avr) {
        disconnectPlugins();
        avr_terminate(avr);
    }

    plugins.clear();
}

void SimAVR::registerPlugin(QSharedPointer<ComponentLogic> plugin)
{
    plugins.append(plugin);
}

void SimAVR::disconnectPlugins()
{
    foreach (QSharedPointer<ComponentLogic> plugin, plugins) {
        plugin->disconnect();
    }
}

void SimAVR::connectPlugins()
{
    foreach (QSharedPointer<ComponentLogic> plugin, plugins) {
        plugin->connect(avr);
    }
}

void SimAVR::load(const QString &filename)
{
    if (avr) {
        disconnectPlugins();
        avr_terminate(avr);
        avr = NULL;
    }

    elf_read_firmware(filename.toStdString().c_str(), &firmware);

    if (firmware.frequency == 0) {
        firmware.frequency = FREQUENCY;
        strcpy(firmware.mmcu, MMCU);
    }

    avr = avr_make_mcu_by_name(firmware.mmcu);
    avr_init(avr);

    avr_load_firmware(avr, &firmware);

    connectPlugins();
}

void SimAVR::run()
{
    avr->state = cpu_Running;
    emit simulationStateChanged(Running);

    avr_reset(avr);

    int state;
    do {
        state = avr_run(avr);
        QCoreApplication::processEvents();
    } while (state != cpu_Done && state != cpu_Crashed);
    emit simulationStateChanged(Done);
}

void SimAVR::pauseSimulation()
{
    avr->state = avr->state == cpu_Stopped ? cpu_Running: cpu_Stopped;
    emit simulationStateChanged(avr->state == cpu_Running ? Running : Paused);
}

void SimAVR::stopSimulation()
{
    avr->state = cpu_Done;
    emit simulationStateChanged(Done);
}

void SimAVR::attachGdb()
{
    /* Only init GDB once. */
    if (avr->gdb_port != GDB_PORT) {
        avr->gdb_port = GDB_PORT;
        avr->state = cpu_Stopped;
        avr_gdb_init(avr);
    } else {
        avr->state = cpu_Stopped;
    }
    emit simulationStateChanged(Paused);
}
