#include "virtualdevices/virtualtreadmill.h"
#include <QThread>
#include <QSettings>
#include <QtMath>
#include <chrono>

#ifdef Q_OS_ANDROID
#include "keepawakehelper.h"
#include <QAndroidJniObject>
#include <QtAndroid>
#endif

using namespace std::chrono_literals;

virtualtreadmill::virtualtreadmill(bluetoothdevice *t, bool noHeartService) {
    QSettings settings;
    treadMill = t;

    this->noHeartService = noHeartService;
    if (settings.value(QZSettings::dircon_yes, QZSettings::default_dircon_yes).toBool()) {
        dirconManager = new DirconManager(t, this);
        connect(dirconManager, SIGNAL(changeInclination(double, double)), this,
                SIGNAL(changeInclination(double, double)));
        connect(dirconManager, SIGNAL(changeInclination(double, double)), this,
                SLOT(dirconChangedInclination(double, double)));
        connect(dirconManager, SIGNAL(ftmsCharacteristicChanged(QLowEnergyCharacteristic, QByteArray)), this,
                SIGNAL(ftmsCharacteristicChanged(QLowEnergyCharacteristic, QByteArray)));
    }
    if (!settings.value(QZSettings::virtual_device_bluetooth, QZSettings::default_virtual_device_bluetooth).toBool())
        return;
    notif2AD9 = new CharacteristicNotifier2AD9(t, this);
    notif2AD2 = new CharacteristicNotifier2AD2(t, this);
    notif2ACD = new CharacteristicNotifier2ACD(t, this);
    notif2A53 = new CharacteristicNotifier2A53(t, this);
    notif2A37 = new CharacteristicNotifier2A37(t, this);
    writeP2AD9 = new CharacteristicWriteProcessor2AD9(t, notif2AD9, this);
    connect(writeP2AD9, SIGNAL(changeInclination(double, double)), this, SIGNAL(changeInclination(double, double)));
    connect(writeP2AD9, SIGNAL(slopeChanged()), this, SLOT(slopeChanged()));
    connect(writeP2AD9, SIGNAL(ftmsCharacteristicChanged(QLowEnergyCharacteristic, QByteArray)), this,
            SIGNAL(ftmsCharacteristicChanged(QLowEnergyCharacteristic, QByteArray)));

    {
        //! [Advertising Data]
        advertisingData.setDiscoverability(QLowEnergyAdvertisingData::DiscoverabilityGeneral);
        advertisingData.setIncludePowerLevel(true);
        advertisingData.setLocalName(QStringLiteral("DomyosBridge"));
        QList<QBluetoothUuid> services;

        // Add Wahoo Run Service UUID
        /*QBluetoothUuid wahooRunServiceUuid(QString("A026EE0E-0A7D-4AB3-97FA-F1500F9FEB8B"));
        services << wahooRunServiceUuid;*/

        if (ftmsServiceEnable()) {
            services << ((QBluetoothUuid::ServiceClassUuid)0x1826); // FitnessMachineServiceUuid
        }

        if (RSCEnable()) {
            services << (QBluetoothUuid::ServiceClassUuid::RunningSpeedAndCadence);
        }

        if (noHeartService == false) {
            services << QBluetoothUuid::HeartRate;
        }

        /*services << ((QBluetoothUuid::ServiceClassUuid)0xFF00);
        services << QBluetoothUuid::DeviceInformation;*/

        advertisingData.setServices(services);
        //! [Advertising Data]

        // Add Device Information Service
        QLowEnergyCharacteristicData manufacturerNameChar;
        manufacturerNameChar.setUuid(QBluetoothUuid::CharacteristicType::ManufacturerNameString);
        manufacturerNameChar.setProperties(QLowEnergyCharacteristic::Read);
        manufacturerNameChar.setValue(QByteArray("Wahoo Fitness")); // Changed to Wahoo Fitness

        QLowEnergyCharacteristicData firmwareRevChar;
        firmwareRevChar.setUuid(QBluetoothUuid::CharacteristicType::FirmwareRevisionString);
        firmwareRevChar.setProperties(QLowEnergyCharacteristic::Read);
        firmwareRevChar.setValue(QByteArray("1.0.11"));

        QLowEnergyCharacteristicData hardwareRevChar;
        hardwareRevChar.setUuid(QBluetoothUuid::CharacteristicType::HardwareRevisionString);
        hardwareRevChar.setProperties(QLowEnergyCharacteristic::Read);
        hardwareRevChar.setValue(QByteArray("1"));

        // Create Device Information Service        
        serviceDataDIS.setType(QLowEnergyServiceData::ServiceTypePrimary);
        serviceDataDIS.setUuid(QBluetoothUuid::DeviceInformation);
        serviceDataDIS.addCharacteristic(manufacturerNameChar);
        serviceDataDIS.addCharacteristic(firmwareRevChar);
        serviceDataDIS.addCharacteristic(hardwareRevChar);

        // Create Wahoo Run Service        
        serviceDataWahoo.setType(QLowEnergyServiceData::ServiceTypePrimary);
        serviceDataWahoo.setUuid(QBluetoothUuid(QString("A026EE0E-0A7D-4AB3-97FA-F1500F9FEB8B")));

        // Add Wahoo Run Notify Characteristic
        QLowEnergyCharacteristicData wahooNotifyChar;
        wahooNotifyChar.setUuid(QBluetoothUuid(QString("A026E03D-0A7D-4AB3-97FA-F1500F9FEB8B")));
        wahooNotifyChar.setProperties(QLowEnergyCharacteristic::Read |
                                    QLowEnergyCharacteristic::WriteNoResponse |
                                    QLowEnergyCharacteristic::Notify);
        const QLowEnergyDescriptorData wahooNotifyConfig(QBluetoothUuid::ClientCharacteristicConfiguration,
                                                        QByteArray(2, 0));
        wahooNotifyChar.addDescriptor(wahooNotifyConfig);

        // Add Wahoo Run Write Characteristic
        QLowEnergyCharacteristicData wahooWriteChar;
        wahooWriteChar.setUuid(QBluetoothUuid(QString("A026E03E-0A7D-4AB3-97FA-F1500F9FEB8B")));
        wahooWriteChar.setProperties(QLowEnergyCharacteristic::WriteNoResponse |
                                   QLowEnergyCharacteristic::Indicate);
        const QLowEnergyDescriptorData wahooWriteConfig(QBluetoothUuid::ClientCharacteristicConfiguration,
                                                       QByteArray(2, 0));
        wahooWriteChar.addDescriptor(wahooWriteConfig);

        serviceDataWahoo.addCharacteristic(wahooNotifyChar);
        serviceDataWahoo.addCharacteristic(wahooWriteChar);

        //! [Service Data]
        if (ftmsServiceEnable()) {
            QLowEnergyCharacteristicData charData;
            charData.setUuid((QBluetoothUuid::CharacteristicType)0x2ACC);  // FitnessMachineFeatureCharacteristicUuid
            QByteArray value;
            value.append(0x08);
            value.append((char)0x14);  // heart rate and elapsed time
            value.append((char)0x00);
            value.append((char)0x00);
            value.append((char)0x00);
            value.append((char)0x00);
            value.append((char)0x00);
            value.append((char)0x00);
            charData.setValue(value);
            charData.setProperties(QLowEnergyCharacteristic::Read);

            /*    const QLowEnergyDescriptorData clientConfig(QBluetoothUuid::ClientCharacteristicConfiguration,
                                                               QByteArray(2, 0));
           charData.addDescriptor(clientConfig);*/

               QLowEnergyCharacteristicData charData2;
               if (ftmsTreadmillEnable()) {
                   charData2.setUuid((QBluetoothUuid::CharacteristicType)0x2ACD); // TreadmillDataCharacteristicUuid
                   charData2.setProperties(QLowEnergyCharacteristic::Notify | QLowEnergyCharacteristic::Read);
                   QByteArray descriptor;
                   descriptor.append((char)0x01);
                   descriptor.append((char)0x00);
                   const QLowEnergyDescriptorData clientConfig2(QBluetoothUuid::ClientCharacteristicConfiguration,
                                                                descriptor);
                   charData2.addDescriptor(clientConfig2);
               }

               QLowEnergyCharacteristicData charData3;
               charData3.setUuid((QBluetoothUuid::CharacteristicType)0x2AD9); // Fitness Machine Control Point
               charData3.setProperties(QLowEnergyCharacteristic::Write | QLowEnergyCharacteristic::Indicate);
               const QLowEnergyDescriptorData cpClientConfig(QBluetoothUuid::ClientCharacteristicConfiguration,
                                                             QByteArray(2, 0));
               charData3.addDescriptor(cpClientConfig);

               QLowEnergyCharacteristicData charDataFIT5;
               charDataFIT5.setUuid((QBluetoothUuid::CharacteristicType)0x2ADA); // Fitness Machine status
               charDataFIT5.setProperties(QLowEnergyCharacteristic::Notify);
               QByteArray descriptor5;
               descriptor5.append((char)0x01);
               descriptor5.append((char)0x00);
               const QLowEnergyDescriptorData clientConfig5(QBluetoothUuid::ClientCharacteristicConfiguration,
                                                            descriptor5);
               charDataFIT5.addDescriptor(clientConfig5);

               QLowEnergyCharacteristicData charDataFIT6;
               charDataFIT6.setUuid((QBluetoothUuid::CharacteristicType)0x2AD3);
               charDataFIT6.setProperties(QLowEnergyCharacteristic::Notify | QLowEnergyCharacteristic::Read);
               QByteArray valueFIT6;
               valueFIT6.append((char)0x00);
               valueFIT6.append((char)0x01);
               charDataFIT6.setValue(valueFIT6);
               QByteArray descriptor6;
               descriptor6.append((char)0x01);
               descriptor6.append((char)0x00);
               const QLowEnergyDescriptorData clientConfig6(QBluetoothUuid::ClientCharacteristicConfiguration,
                                                            descriptor6);
               charDataFIT6.addDescriptor(clientConfig6);
               charDataFIT6.setProperties(QLowEnergyCharacteristic::Read);

               // indoor bike in order to exploit a zwift bug that allows treadmill to get the incline values
               QLowEnergyCharacteristicData charDataFIT7;
               charDataFIT7.setUuid((QBluetoothUuid::CharacteristicType)0x2AD2); // indoor bike
               charDataFIT7.setProperties(QLowEnergyCharacteristic::Notify | QLowEnergyCharacteristic::Read);
               QByteArray descriptor7;
               descriptor7.append((char)0x01);
               descriptor7.append((char)0x00);
               const QLowEnergyDescriptorData clientConfig7(QBluetoothUuid::ClientCharacteristicConfiguration,
                                                            descriptor7);
               charDataFIT7.addDescriptor(clientConfig7);

               QLowEnergyCharacteristicData charDataFIT2;
               charDataFIT2.setUuid(
                   (QBluetoothUuid::CharacteristicType)0x2AD6); // supported_resistance_level_rangeCharacteristicUuid
               charDataFIT2.setProperties(QLowEnergyCharacteristic::Read);
               QByteArray valueFIT2;
               valueFIT2.append((char)0x0A); // min resistance value
               valueFIT2.append((char)0x00); // min resistance value
               valueFIT2.append((char)0x96); // max resistance value
               valueFIT2.append((char)0x00); // max resistance value
               valueFIT2.append((char)0x0A); // step resistance
               valueFIT2.append((char)0x00); // step resistance
               charDataFIT2.setValue(valueFIT2);

               serviceDataFTMS.setType(QLowEnergyServiceData::ServiceTypePrimary);
               serviceDataFTMS.setUuid((QBluetoothUuid::ServiceClassUuid)0x1826); // FitnessMachineServiceUuid
               serviceDataFTMS.addCharacteristic(charData);
               if (ftmsTreadmillEnable())
                   serviceDataFTMS.addCharacteristic(charData2);
               serviceDataFTMS.addCharacteristic(charData3);
               serviceDataFTMS.addCharacteristic(charDataFIT5);
               serviceDataFTMS.addCharacteristic(charDataFIT6);
               serviceDataFTMS.addCharacteristic(charDataFIT7);
               serviceDataFTMS.addCharacteristic(charDataFIT2);
           }


           if (RSCEnable()) {
               QLowEnergyCharacteristicData charData;
               charData.setUuid(QBluetoothUuid::CharacteristicType::RSCFeature);
               charData.setProperties(QLowEnergyCharacteristic::Read);
               QByteArray value;
               value.append((char)0x02); // Total Distance Measurement Supported
               value.append((char)0x00);
               charData.setValue(value);

               QLowEnergyCharacteristicData charData2;
               charData2.setUuid(QBluetoothUuid::CharacteristicType::SensorLocation);
               charData2.setProperties(QLowEnergyCharacteristic::Read);
               QByteArray valueLocaltion;
               valueLocaltion.append((char)1); // Top of shoe
               charData2.setValue(valueLocaltion);
               /*const QLowEnergyDescriptorData clientConfig2(QBluetoothUuid::ClientCharacteristicConfiguration,
                                                           QByteArray(2, 0));
               charData2.addDescriptor(clientConfig2);*/

               QLowEnergyCharacteristicData charData3;
               charData3.setUuid(QBluetoothUuid::CharacteristicType::RSCMeasurement);
               charData3.setProperties(QLowEnergyCharacteristic::Read | QLowEnergyCharacteristic::Notify);
               charData3.setValue(valueLocaltion);
               QByteArray descriptor;
               descriptor.append((char)0x01);
               descriptor.append((char)0x00);
               const QLowEnergyDescriptorData clientConfig4(QBluetoothUuid::ClientCharacteristicConfiguration, descriptor);
               charData3.addDescriptor(clientConfig4);

               QLowEnergyCharacteristicData charData4;
               charData4.setUuid(QBluetoothUuid::CharacteristicType::SCControlPoint);
               charData4.setProperties(QLowEnergyCharacteristic::Write | QLowEnergyCharacteristic::Indicate);
               const QLowEnergyDescriptorData cpClientConfig(QBluetoothUuid::ClientCharacteristicConfiguration,
                                                             QByteArray(2, 0));
               charData4.addDescriptor(cpClientConfig);

               serviceDataRSC.setType(QLowEnergyServiceData::ServiceTypePrimary);
               serviceDataRSC.setUuid(QBluetoothUuid::ServiceClassUuid::RunningSpeedAndCadence);
               serviceDataRSC.addCharacteristic(charData);
               serviceDataRSC.addCharacteristic(charData3);
               serviceDataRSC.addCharacteristic(charData2);
               serviceDataRSC.addCharacteristic(charData4);
           }
           //! [Service Data]

           if (noHeartService == false) {
               QLowEnergyCharacteristicData charDataHR;
               charDataHR.setUuid(QBluetoothUuid::HeartRateMeasurement);
               charDataHR.setValue(QByteArray(2, 0));
               charDataHR.setProperties(QLowEnergyCharacteristic::Notify);
                const QLowEnergyDescriptorData clientConfigHR(QBluetoothUuid::ClientCharacteristicConfiguration,
                                                              QByteArray(2, 0));
                charDataHR.addDescriptor(clientConfigHR);

                serviceDataHR.setType(QLowEnergyServiceData::ServiceTypePrimary);
                serviceDataHR.setUuid(QBluetoothUuid::HeartRate);
                serviceDataHR.addCharacteristic(charDataHR);
            }

        //! [Start Advertising]
        leController = QLowEnergyController::createPeripheral();
        Q_ASSERT(leController);

        if (ftmsServiceEnable())
            serviceFTMS = leController->addService(serviceDataFTMS);
        QThread::msleep(100);
        
        if (RSCEnable())
            serviceRSC = leController->addService(serviceDataRSC);
        QThread::msleep(100);
        
        serviceDIS = leController->addService(serviceDataDIS);
        QThread::msleep(100);
        
        serviceWahoo = leController->addService(serviceDataWahoo);
        QThread::msleep(100);

        
        if (noHeartService == false) {
            serviceHR = leController->addService(serviceDataHR);
        }

        if (serviceFTMS)
            QObject::connect(serviceFTMS, &QLowEnergyService::characteristicChanged, this,
                             &virtualtreadmill::characteristicChanged);

        if (serviceWahoo)
            QObject::connect(serviceWahoo, &QLowEnergyService::characteristicChanged, this,
                             &virtualtreadmill::wahooCharacteristicChanged);

        QLowEnergyAdvertisingParameters pars = QLowEnergyAdvertisingParameters();
            pars.setInterval(100, 100);

#ifdef Q_OS_ANDROID
        QAndroidJniObject::callStaticMethod<void>("org/cagnulen/qdomyoszwift/BleAdvertiser",
                                                 "startAdvertisingTreadmill",
                                                 "(Landroid/content/Context;)V",
                                                 QtAndroid::androidContext().object());

#endif
        //! [Start Advertising]

        QObject::connect(leController, &QLowEnergyController::disconnected, this, &virtualtreadmill::reconnect);
    }

    QObject::connect(&treadmillTimer, &QTimer::timeout, this, &virtualtreadmill::treadmillProvider);
    treadmillTimer.start(1s);
}

void virtualtreadmill::characteristicChanged(const QLowEnergyCharacteristic &characteristic,
                                                const QByteArray &newValue) {
    qDebug() << QStringLiteral("characteristicChanged ") + QString::number(characteristic.uuid().toUInt16()) +
                    QStringLiteral(" ") << newValue.toHex(' ');
    QByteArray reply;

    switch (characteristic.uuid().toUInt16()) {
    case 0x2AD9: // Fitness Machine Control Point
        if (writeP2AD9->writeProcess(0x2AD9, newValue, reply) == CP_OK) {

            QLowEnergyCharacteristic characteristic =
                serviceFTMS->characteristic((QBluetoothUuid::CharacteristicType)0x2AD9);
            Q_ASSERT(characteristic.isValid());
            if (leController->state() != QLowEnergyController::ConnectedState) {
                qDebug() << QStringLiteral("virtual treadmill not connected");
    
                return;
            }
            try {
                qDebug() << QStringLiteral("virtualtreadmill::writeCharacteristic ") + serviceFTMS->serviceName() +
                                QStringLiteral(" ") + characteristic.name() + QStringLiteral(" ") + reply.toHex(' ');
                serviceFTMS->writeCharacteristic(characteristic, reply); // Potentially causes notification.
            } catch (...) {
                qDebug() << QStringLiteral("virtual treadmill error!");
            }
        }
        break;
    }
}

void virtualtreadmill::slopeChanged() {
    lastSlopeChanged = QDateTime::currentSecsSinceEpoch();
    
    if (treadMill && treadMill->autoResistance())
        m_autoInclinationEnabled = true;
    else
        m_autoInclinationEnabled = false;

}

void virtualtreadmill::wahooCharacteristicChanged(const QLowEnergyCharacteristic &characteristic,
                                                const QByteArray &newValue) {
    qDebug() << "Wahoo characteristic changed: " << characteristic.uuid().toString()
             << " value: " << newValue.toHex(' ');
    
    // Handle Wahoo characteristic changes here
    if (characteristic.uuid() == QBluetoothUuid(QString("A026E03D-0A7D-4AB3-97FA-F1500F9FEB8B"))) {
        // Handle notify characteristic
    } else if (characteristic.uuid() == QBluetoothUuid(QString("A026E03E-0A7D-4AB3-97FA-F1500F9FEB8B"))) {
        if(newValue.length() >= 3) {
              if(newValue.at(0) == 0x11) {
                  int16_t slope = (((uint8_t)newValue.at(1)) + (newValue.at(2) << 8));
                  writeP2AD9->changeSlope(slope, 0, 0);
              }
          }
    }
}

void virtualtreadmill::reconnect() {
    qDebug() << "virtualtreadmill reconnect " << treadMill->connected();
    
    if (ftmsServiceEnable())
        serviceFTMS = leController->addService(serviceDataFTMS);
    QThread::msleep(100);
    
    if (RSCEnable())
        serviceRSC = leController->addService(serviceDataRSC);
    QThread::msleep(100);
    
    serviceDIS = leController->addService(serviceDataDIS);
    QThread::msleep(100);
    
    serviceWahoo = leController->addService(serviceDataWahoo);
    QThread::msleep(100);

    
    if (noHeartService == false) {
        serviceHR = leController->addService(serviceDataHR);
    }

    QLowEnergyAdvertisingParameters pars;
    pars.setInterval(100, 100);

    if (serviceFTMS || serviceRSC || serviceWahoo) {
#ifdef Q_OS_ANDROID
        QAndroidJniObject::callStaticMethod<void>("org/cagnulen/qdomyoszwift/BleAdvertiser",
                                                 "startAdvertisingTreadmill",
                                                 "(Landroid/content/Context;)V",
                                                 QtAndroid::androidContext().object());
#endif
    }
}


void virtualtreadmill::treadmillProvider() {
    const uint64_t slopeTimeoutSecs = 30;
    QSettings settings;

    if ((uint64_t)QDateTime::currentSecsSinceEpoch() > lastSlopeChanged + slopeTimeoutSecs)
        m_autoInclinationEnabled = false;

    if (leController->state() != QLowEnergyController::ConnectedState) {
        qDebug() << QStringLiteral("virtualtreadmill connection error");
        return;
    }

    QByteArray value;

    if (ftmsServiceEnable()) {
        if (ftmsTreadmillEnable()) {
            value.clear();
            if (notif2ACD->notify(value) == CN_OK) {
                if (!serviceFTMS) {
                    qDebug() << QStringLiteral("service not available");

                    return;
                }

                QLowEnergyCharacteristic characteristic =
                    serviceFTMS->characteristic((QBluetoothUuid::CharacteristicType)0x2ACD);
                Q_ASSERT(characteristic.isValid());
                if (leController->state() != QLowEnergyController::ConnectedState) {
                    qDebug() << QStringLiteral("virtual treadmill not connected");

                    return;
                }
                try {
                    serviceFTMS->writeCharacteristic(characteristic, value); // Potentially causes notification.
                } catch (...) {
                    qDebug() << QStringLiteral("virtualtreadmill error!");
                }
            }
        }
        value.clear();
        if (notif2AD2->notify(value) == CN_OK) {
            if (!serviceFTMS) {
                qDebug() << QStringLiteral("serviceFIT not available");

                return;
            }

            QLowEnergyCharacteristic characteristic =
                serviceFTMS->characteristic((QBluetoothUuid::CharacteristicType)0x2AD2);
            Q_ASSERT(characteristic.isValid());
            if (leController->state() != QLowEnergyController::ConnectedState) {
                qDebug() << QStringLiteral("virtual treadmill not connected");

                return;
            }
            try {
                serviceFTMS->writeCharacteristic(characteristic, value); // Potentially causes notification.
            } catch (...) {
                qDebug() << QStringLiteral("virtualtreadmill error!");
            }
        }
    }
    if (RSCEnable()) {
        value.clear();
        if (notif2A53->notify(value) == CN_OK) {
            if (!serviceRSC) {
                qDebug() << QStringLiteral("serviceFIT not available");

                return;
            }

            QLowEnergyCharacteristic characteristic =
                serviceRSC->characteristic(QBluetoothUuid::CharacteristicType::RSCMeasurement);
            Q_ASSERT(characteristic.isValid());
            if (leController->state() != QLowEnergyController::ConnectedState) {
                qDebug() << QStringLiteral("virtual treadmill not connected");

                return;
            }
            try {
                serviceRSC->writeCharacteristic(characteristic, value); // Potentially causes notification.
            } catch (...) {
                qDebug() << QStringLiteral("virtualtreadmill error!");
            }
        }
    }

    // characteristic
    //        = service->characteristic((QBluetoothUuid::CharacteristicType)0x2AD9); // Fitness Machine Control Point
    // Q_ASSERT(characteristic.isValid());
    // service->readCharacteristic(characteristic);

    if (noHeartService == false) {
        value.clear();
        if (notif2A37->notify(value) == CN_OK) {
            if (!serviceHR) {
                qDebug() << QStringLiteral("serviceFIT not available");

                return;
            }

            QLowEnergyCharacteristic characteristic =
                serviceHR->characteristic(QBluetoothUuid::CharacteristicType::HeartRateMeasurement);
            Q_ASSERT(characteristic.isValid());
            if (leController->state() != QLowEnergyController::ConnectedState) {
                qDebug() << QStringLiteral("virtual treadmill not connected");

                return;
            }
            try {
                serviceHR->writeCharacteristic(characteristic, value); // Potentially causes notification.
            } catch (...) {
                qDebug() << QStringLiteral("virtualtreadmill error!");
            }
        }
    }
}

bool virtualtreadmill::connected() {
    if (!leController) {
        return false;
    }
    return leController->state() == QLowEnergyController::ConnectedState;
}

bool virtualtreadmill::ftmsServiceEnable() { return true; }

bool virtualtreadmill::ftmsTreadmillEnable() { return true; }

bool virtualtreadmill::RSCEnable() { return false; }

void virtualtreadmill::dirconChangedInclination(double grade, double percentage) {
    Q_UNUSED(grade);
    Q_UNUSED(percentage);
    slopeChanged();
}
