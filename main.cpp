#include "QtWidgetsApplication1.h"
#include <QtWidgets/QApplication>
#include <fastcdr/cdr/fixed_size_string.hpp>
#include <fastcdr/xcdr/optional.hpp>
#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/subscriber/DataReaderListener.hpp>
#include <fastrtps/subscriber/SampleInfo.h>
#include <fastdds/dds/core/status/SubscriptionMatchedStatus.hpp>

using namespace eprosima::fastdds::dds;

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QtWidgetsApplication1 w;
    app.setWindowIcon(QIcon(":/QtWidgetsApplication1/res/spreadsheet.png"));
    w.show();
    return app.exec();
}
