#include <QCoreApplication>
#include <QRandomGenerator>
#include <stdio.h>
#include <string.h>
#include <QDebug>
#include <QThread>

extern "C" {
    #include "../../../Libs/sheller/Source/sheller.h"
}

sheller_t shell;

#define TEST_START_BYTE 0x23
#define TEST_DATA_LENGTH 16
#define TEST_RX_BUFF_LENGTH 256

void test_full();

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    qDebug() << "App start";

    test_full();

    qDebug() << "\nApp end";
    return a.exec();
}

void test_full()
{
    qDebug() << "\nStart full test";
    bool sheller_init_result = sheller_init(&shell, TEST_START_BYTE, TEST_DATA_LENGTH, TEST_RX_BUFF_LENGTH);
    if (sheller_init_result == false) {
        qDebug() << "sheller init result return false";
        qDebug() << "Cannot start the test";
        return;
    }

    uint64_t iteration_counter = 0;
    while(1)
    {
        qDebug() << "Begin test iteration " << QString::number(iteration_counter++) << "--------------------------------";
        uint8_t random_state = QRandomGenerator::global()->bounded(2); //0, 1, 2
        qDebug() << "Test case " << QString::number(random_state);

        uint8_t user_message[TEST_DATA_LENGTH] = {0};
        for (int i = 0; i < TEST_DATA_LENGTH; ++i) {
            user_message[i] = QRandomGenerator::global()->bounded(256);
        }
        qDebug() << "Generated user message:    " << QByteArray((char*)user_message, TEST_DATA_LENGTH).toHex('.');

        uint8_t user_message_wrapered[128] = {0};
        uint8_t sheller_wrap_result = sheller_wrap(&shell, user_message, TEST_DATA_LENGTH, user_message_wrapered);
        if (sheller_wrap_result == SHELLER_ERROR) {
            qDebug() << "[ ERROR ] sheller_wrap return false";
            qDebug() << "Stopping test...";
            return;
        }
        qDebug() << "Wrappered user message: " << QByteArray((char*)user_message_wrapered, 11).toHex('.');

        //Emulate normal packages
        if (random_state == 1) {

            for (int i = 0; i < sheller_get_package_length(&shell); ++i) {
                if (sheller_push(&shell, user_message_wrapered[i]) == SHELLER_ERROR) {
                    qDebug() << "[ ERROR ] Sheller circular buffer overflow";
                    qDebug() << "Stopping test...";
                    return;
                }
            }

            uint8_t received_message[TEST_DATA_LENGTH] = {0};
            while(sheller_read(&shell, received_message) == SHELLER_ERROR) {
                qDebug() << "Wait read in main";
            }

            uint8_t sheller_read_result = SHELLER_OK; //Костыль
            if (sheller_read_result == SHELLER_OK) {
                qDebug() << "Receive message:           " << QByteArray((char*)received_message, TEST_DATA_LENGTH).toHex('.');
                if(strncmp((const char*)user_message, (const char*)received_message, TEST_DATA_LENGTH) == 0) {
                    qDebug() << "Behaviour ok";
                } else {
                    qDebug() << "\nBefore\nSheller buff: " << QByteArray((char*)shell.rx_buff, TEST_RX_BUFF_LENGTH).toHex('.');
                    qDebug() << "[ ERROR ] Not equil generated and received data";
                    qDebug() << "Stopping test...";
                    return;
                }
            } else {
                qDebug() << "sheller_read return false in normal case";
                qDebug() << "Stopping test...";
                return;
            }
        }

        //Emulate damaged packages
        else if (random_state == 0) {
            uint8_t count = QRandomGenerator::global()->bounded(1, TEST_DATA_LENGTH - 1); //!!!!!!!!!!!
            for (uint8_t i = 0; i < count; ++i) {
                if (sheller_push(&shell, user_message[i]) == SHELLER_ERROR) {
                    qDebug() << "[ ERROR ] Sheller circular buffer overflow";
                    qDebug() << "Stopping test...";
                    return;
                }
            }
        }

        //QThread().currentThread()->msleep(1);
        qDebug() << "End iteration\n";
    }
    qDebug() << "\nEnd full test --------------------------------------------------------------------------";
}
