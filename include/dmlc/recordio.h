/*!
 *  Copyright (c) 2015 by Contributors
 * \file recordio.h
 * \brief recordio at 
 */
#ifndef DMLC_RECORDIO_H_
#define DMLC_RECORDIO_H_
#include <string>
#include <cstring>
#include "./io.h"
#include "./logging.h"

namespace dmlc {
/*!
 * \brief writer of binary recordio
 *  binary format for recordio
 *  recordio format: magic lrecord data pad
 *
 *  - magic is magic number
 *  - pad is simply a padding space to make record align to 4 bytes
 *  - lrecord encodes length and continue bit
 *     - data.length() = (lrecord & (1U<<30U - 1));
 *     - cflag == (lrecord >> 30U) & 3;
 *
 *  cflag was used to handle (rare) special case when magic number
 *  occured in the data sequence.
 *
 *  In such case, the data is splitted into multiple records by
 *  the cells of magic number
 *
 *  (1) cflag == 0: this is a complete record;
 *  (2) cflag == 1: start of a multiple-rec;
 *      cflag == 2: middle of multiple-rec; 
 *      cflag == 3: end of multiple-rec
 */
class RecordIOWriter {  
 public:
  /*! \brief magic number of recordio */
  static const unsigned kMagic = 0x3ed7230a;
  /*!
   * \brief encode the lrecord
   * \param cflag cflag part of the lrecord
   * \param length length part of lrecord
   */
  inline static unsigned EncodeLRec(unsigned cflag, unsigned length) {
    return (cflag << 30U) | length;
  }
  /*!
   * \brief decode the flag part of lrecord
   * \param rec the lrecord
   * \return the flag
   */
  inline static unsigned DecodeFlag(unsigned rec) {
    return (rec >> 30U) & 3U;
  }
  /*!
   * \brief decode the length part of lrecord
   * \param rec the lrecord
   * \return the length
   */
  inline static unsigned DecodeLength(unsigned rec) {
    return rec & ((1U << 30U) - 1U);
  }
  /*!
   * \brief constructor
   * \param stream the stream to be constructed
   */
  explicit RecordIOWriter(Stream *stream)
      : stream_(stream), except_counter_(0) { 
    CHECK(sizeof(unsigned) == 4) << "unsigned needs to be 4 bytes";
  }
  /*!
   * \brief write record to the stream
   * \param buf the buffer of memory region
   * \param size the size of record to write out
   */
  void WriteRecord(const void *buf, size_t size);
  /*!
   * \brief write record to the stream
   * \param data the data to write out
   */
  inline void WriteRecord(const std::string &data) {
    this->WriteRecord(data.c_str(), data.length());
  }
  /*!
   * \return number of exceptions(occurance of magic number)
   *   during the writing process
   */
  inline unsigned except_counter(void) const {
    return except_counter_;
  }

 private:
  /*! \brief output stream */
  Stream *stream_;
  /*! \brief counts the number of exceptions */
  unsigned except_counter_;
};
/*!
 * \brief reader of binary recordio to reads in record
 * \sa RecordIOWriter
 */
class RecordIOReader {
 public:
  /*!
   * \brief constructor
   * \param stream the stream to be constructed
   */
  explicit RecordIOReader(Stream *stream)
      : stream_(stream), end_of_stream_(false) {
    CHECK(sizeof(unsigned) == 4) << "unsigned needs to be 4 bytes";
  }
  /*!
   * \brief read next complete record from stream   
   * \param out_rec used to store output record in string
   * \return true of read was successful, false if end of stream was reached
   */
  bool ReadRecord(std::string *out_rec);

 private:
  /*! \brief output stream */
  Stream *stream_;
  /*! \brief whether we are at end of stream */
  bool end_of_stream_;
};
}  // namespace dmlc
#endif  // DMLC_RECORDIO_H_
