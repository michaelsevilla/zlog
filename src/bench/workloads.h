#ifndef ZLOG_SRC_BENCH_WORKLOADS_H
#define ZLOG_SRC_BENCH_WORKLOADS_H

//#define BENCH_DEBUG

/*
 * MapN1:
 *  - a log entry maps to one of N distinct objects (round-robin)
 *  - the log entry is stored in obj.omap[seq]
 *
 * Mapping:
 *  - log[seq] => obj.[seq % stripe_width].omap[seq]
 */
class MapN1_Workload : public Workload {
 public:
  MapN1_Workload(librados::IoCtx *ioctx, size_t stripe_width,
      size_t entry_size, int qdepth, OpHistory *op_history,
      std::string& prefix) :
    Workload(op_history, qdepth, entry_size, prefix),
    ioctx_(ioctx),
    stripe_width_(stripe_width)
  {}

  void gen_op(librados::AioCompletion *rc, uint64_t *submitted_ns,
      ceph::bufferlist& bl) {

    // target object (e.g. seq=127 => prefix.log_mapN1.3.omap[127])
    std::stringstream oid;
    size_t stripe_index = seq % stripe_width_;
    oid << prefix_ << "log_mapN1." << stripe_index;

    // target omap key (key = seq)
    std::stringstream key;
    key << seq;

    // omap set op
    librados::ObjectWriteOperation op;
    std::map<std::string, ceph::bufferlist> kvs;
    kvs[key.str()] = bl;
    op.omap_set(kvs);

#ifdef BENCH_DEBUG
    std::stringstream kvs_dump;

    for (auto& entry : kvs) {
      kvs_dump << "key=" << entry.first << " "
               << "val=bl/" << entry.second.length() << " ";
    }

    std::cout << "workload=mapN1" << " "
              << "seq=" << seq << " "
              << "obj=" << oid.str() << " "
              << "omap_set: " << kvs_dump.str()
              << std::endl;
#endif

    //  submit the io
    *submitted_ns = getns();
    int ret = ioctx_->aio_operate(oid.str(), rc, &op);
    assert(ret == 0);
  }

 private:
  librados::IoCtx *ioctx_;
  size_t stripe_width_;
};

/*
 * Map11:
 *  - a log entry maps to a distinct object
 *  - the log entry is stored in obj.omap["entry"]
 *
 * Mapping:
 *  - log[seq] => obj.seq.omap["entry"]
 */
class Map11_Workload : public Workload {
 public:
  Map11_Workload(librados::IoCtx *ioctx, size_t entry_size,
      int qdepth, OpHistory *op_history, std::string& prefix) :
    Workload(op_history, qdepth, entry_size, prefix),
    ioctx_(ioctx)
  {}

  void gen_op(librados::AioCompletion *rc, uint64_t *submitted_ns,
      ceph::bufferlist& bl) {

    // target object (e.g. seq=127 => prefix.log_map11.127)
    std::stringstream oid;
    oid << prefix_ << "log_map11." << seq;

    // omap set op
    librados::ObjectWriteOperation op;
    std::map<std::string, ceph::bufferlist> kvs;
    kvs["entry"] = bl;
    op.omap_set(kvs);

#ifdef BENCH_DEBUG
    std::stringstream kvs_dump;

    for (auto& entry : kvs) {
      kvs_dump << "key=" << entry.first << " "
               << "val=bl/" << entry.second.length() << " ";
    }

    std::cout << "workload=map11" << " "
              << "seq=" << seq << " "
              << "obj=" << oid.str() << " "
              << "omap_set: " << kvs_dump.str()
              << std::endl;
#endif

    //  submit the io
    *submitted_ns = getns();
    int ret = ioctx_->aio_operate(oid.str(), rc, &op);
    assert(ret == 0);
  }

 private:
  librados::IoCtx *ioctx_;
};

/*
 * ByteStream11:
 *  - a log entry maps to a distinct object
 *  - the log entry is stored in obj.write(...)
 *
 * Mapping:
 *  - log[seq] => obj.seq.write(...)
 */
class ByteStream11_Workload : public Workload {
 public:
  ByteStream11_Workload(librados::IoCtx *ioctx, size_t entry_size,
      int qdepth, OpHistory *op_history, std::string& prefix) :
    Workload(op_history, qdepth, entry_size, prefix),
    ioctx_(ioctx)
  {}

  void gen_op(librados::AioCompletion *rc, uint64_t *submitted_ns,
      ceph::bufferlist& bl) {

    // target object (e.g. seq=127 => prefix.log_bytestream11.127)
    std::stringstream oid;
    oid << prefix_ << "log_bytestream11." << seq;

#ifdef BENCH_DEBUG
    std::cout << "workload=bytestream11" << " "
              << "seq=" << seq << " "
              << "obj=" << oid.str() << " "
              << "off=0 (write_full)" << " "
              << "data=bl/" << bl.length()
              << std::endl;
#endif

    //  submit the io
    *submitted_ns = getns();
    int ret = ioctx_->aio_write_full(oid.str(), rc, bl);
    assert(ret == 0);
  }

 private:
  librados::IoCtx *ioctx_;
};

/*
 * ByteStreamN1Write:
 *  - a log entry maps to one of N distinct objects (round-robin)
 *  - the log entry is stored at a fixed offset in the object
 *
 * Mapping:
 *  - select object: log[seq] => obj.[seq % stripe_width]
 *      - select offset: obj.write(seq / stripe_width * entry_size)
 */
class ByteStreamN1Write_Workload : public Workload {
 public:
  ByteStreamN1Write_Workload(librados::IoCtx *ioctx, size_t stripe_width,
      size_t entry_size, int qdepth, OpHistory *op_history,
      std::string& prefix) :
    Workload(op_history, qdepth, entry_size, prefix),
    ioctx_(ioctx),
    stripe_width_(stripe_width)
  {}

  void gen_op(librados::AioCompletion *rc, uint64_t *submitted_ns,
      ceph::bufferlist& bl) {

    // target object (e.g. seq=127 => prefix.log_mapN1.3)
    std::stringstream oid;
    size_t stripe_index = seq % stripe_width_;
    oid << prefix_ << "log_bytestreamN1write." << stripe_index;

    // compute offset within object
    uint64_t offset = seq / stripe_width_ * entry_size_;

#ifdef BENCH_DEBUG
    std::cout << "workload=bytestreamN1write" << " "
              << "seq=" << seq << " "
              << "obj=" << oid.str() << " "
              << "off=" << offset << " "
              << "data=bl/" << bl.length()
              << std::endl;
#endif
    
    //  submit the io
    *submitted_ns = getns();
    int ret = ioctx_->aio_write(oid.str(), rc, bl, bl.length(), offset);
    assert(ret == 0);
  }

 private:
  librados::IoCtx *ioctx_;
  size_t stripe_width_;
};

/*
 * ByteStreamN1Append:
 *  - a log entry maps to one of N distinct objects (round-robin)
 *  - the log entry is append to the object
 *
 * Mapping:
 *  - select object: log[seq] => obj.[seq % stripe_width]
 *  - perform append
 */
class ByteStreamN1Append_Workload : public Workload {
 public:
  ByteStreamN1Append_Workload(librados::IoCtx *ioctx, size_t stripe_width,
      size_t entry_size, int qdepth, OpHistory *op_history,
      std::string& prefix) :
    Workload(op_history, qdepth, entry_size, prefix),
    ioctx_(ioctx),
    stripe_width_(stripe_width)
  {}

  void gen_op(librados::AioCompletion *rc, uint64_t *submitted_ns,
      ceph::bufferlist& bl) {

    // target object (e.g. seq=127 => prefix.log_mapN1.3)
    std::stringstream oid;
    size_t stripe_index = seq % stripe_width_;
    oid << prefix_ << "log_bytestreamN1append." << stripe_index;

#ifdef BENCH_DEBUG
    std::cout << "workload=bytestreamN1append" << " "
              << "seq=" << seq << " "
              << "obj=" << oid.str() << " "
              << "off=? (append)" <<  " "
              << "data=bl/" << bl.length()
              << std::endl;
#endif
    
    //  submit the io
    *submitted_ns = getns();
    int ret = ioctx_->aio_append(oid.str(), rc, bl, bl.length());
    assert(ret == 0);
  }

 private:
  librados::IoCtx *ioctx_;
  size_t stripe_width_;
};

#endif
