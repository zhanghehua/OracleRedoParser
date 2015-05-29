#ifndef APPLIER_INC
#define APPLIER_INC
#include <map>
#include <string>
#include <memory>
#include <utility>
#include <list>
#define OTL_ORA11G_R2
#define OTL_ORA_UTF8
#include "otlv4.h"
#include "trans.h"
#include "stream.h"

namespace databus {
  const std::list<std::pair<std::string, std::string>> prefix_cols{
      std::make_pair("STREAM_XID", "to_number(:stream_xid<char[39]>)"),
      std::make_pair("STREAM_OP", ":stream_op<char[40]>"),
      std::make_pair(
          "STREAM_SCN",
          "to_number(:stream_scn<char[29]>, 'XXXXXXXXXXXXXXXXXXXXXXXXXXXX')"),
      std::make_pair(
          "STREAM_TIMESTAMP",
          "to_date(:stream_timestamp<char[21]>, 'yyyy-mm-dd hh24:mi:ss')")};
  class TabDef;
  typedef std::shared_ptr<TabDef> TabDefPtr;
  class SimpleApplier {
    // Only recover PKs
   public:
    // add table statement into stmt_dict_,
    // if false == true, it will overwrite the the previous data
    void addTable(TabDefPtr tab_def, bool force = false);
    void apply(TransactionPtr tran_ptr);
    static SimpleApplier& getApplier(const char* conn_str) {
      static SimpleApplier applier(conn_str);
      return applier;
    }

   private:
    SimpleApplier(const char* conn_str);
    std::string getInsertStmt(TabDefPtr tab_def);
    void _apply(RowChangePtr rcp, TabDefPtr tab_def, XID xid, char offset);

   private:
    std::string conn_str_;
    otl_connect conn_;
    std::map<std::string, std::shared_ptr<otl_stream>> stmt_dict_;
  };

  struct ApplyStats {
    ApplyStats() {}
    ApplyStats(const TimePoint& restart_tp, const TimePoint& commit_tp)
        : restart_tp_(restart_tp), commit_tp_(commit_tp) {}
    TimePoint restart_tp_;
    TimePoint commit_tp_;
  };

  class ApplierHelper {
    // record/get the apply progress into/from database

   public:
    ApplyStats getApplyStats();
    void saveApplyProgress(const TimePoint& commit_tp,
                           const TimePoint& restart_tp);
    static ApplierHelper& getApplierHelper() {
      static ApplierHelper applierHelper(
          streamconf->getString("tarConn").c_str(),
          streamconf->getUint32("instId"));
      return applierHelper;
    }

   private:
    ApplierHelper(const char* conn_str, uint32_t inst_id);

   private:
    uint32_t inst_id_;
    otl_connect conn_;
    otl_stream save_progress_stmt_;
    otl_stream get_progress_stmt_;
  };
}
#endif /* ----- #ifndef APPLIER_INC  ----- */
