#include "anchor.hpp"
#include "anchor_link.hpp"
#include "argparser.hpp"
#include "diagnose.hpp"
#include "firmware_manager.hpp"
#include "kafka_client.hpp"
#include "log.hpp"
#include "msg_handle.hpp"
#include "positioning.hpp"
#include "spdlog/spdlog.h"
#include "tag.hpp"
#include <QCoreApplication>

class Application : public QCoreApplication
{
public:
  using QCoreApplication::QCoreApplication;
  bool notify(QObject *receiver, QEvent *event) override
  {
    try
    {
      return QCoreApplication::notify(receiver, event);
    }
    catch (const std::exception &e)
    {
      spdlog::error("in event handle, uncatched std exception occured:{}",
                    e.what());
    }
    catch (...)
    {
      spdlog::error("in event handle, uncatched non std exception "
                    "occured,program exited");
    }
    abort();
  }
};

int main(int argc, char *argv[])
{
  int ret_code = EXIT_FAILURE;
#ifdef QT_DEBUG
  QCoreApplication app(argc, argv);
#else
  Application app(argc, argv);
#endif
  auto init = []
  {
    qApp->setOrganizationName("Nooploop");
    qApp->setOrganizationDomain("www.nooploop.com");
    qApp->setApplicationVersion("1.0.5");

    arg::init(qApp->arguments());
    log_init(arg::log_level(), arg::log_dir());

    std::unique_ptr<AnchorManager> anchor_manager(AnchorManager::create_ins());
    std::unique_ptr<TagManager> tag_manager(TagManager::create_ins());

    std::unique_ptr<MsgHandle> msg_handle(MsgHandle::create_ins());

    std::unique_ptr<KafkaClient> kafka_client(KafkaClient::create_ins());

    kafka_client->init(arg::kafka_addr(), arg::log_level(),
                       msg_handle->topics());

    std::unique_ptr<Diagnose> diagnose(Diagnose::create_ins());
    std::unique_ptr<Positioning> positioning(Positioning::create_ins());
    std::unique_ptr<FirmwareManager> firmware_manager(
        FirmwareManager::create_ins());

    std::unique_ptr<AnchorLinkManager> anchor_link_manager(
        AnchorLinkManager::create_ins());

    kafka_client->wait_connected();
    anchor_link_manager->init(arg::anchor_link_port());

    return qApp->exec();
  };
#ifdef QT_DEBUG
  ret_code = init();
#else
  try
  {
    ret_code = init();
  }
  catch (const std::exception &e)
  {
    auto info = fmt::format("uncatched std exception occured:{}", e.what());
    spdlog::error(info);
    std::cerr << info << std::endl;
  }
  catch (...)
  {
    auto info =
        std::string("uncatched non std exception occured,program exited");
    spdlog::error(info);
    std::cerr << info << std::endl;
  }
#endif
  // FIXME 即使这里调用了flush 也不会打印
  spdlog::default_logger()->flush();
  return ret_code;
}
