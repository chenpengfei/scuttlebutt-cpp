//
// Created by 陈鹏飞 on 2020/2/29.
//

#ifndef SCUTTLEBUTT_DUPLEX_STREAM_H
#define SCUTTLEBUTT_DUPLEX_STREAM_H

#include "event-emitter/include/event_emitter.h"

// A stream is an abstract interface for working with streaming data.
// The stream module provides an API for implementing the stream interface.

// ## Buffering
// Stream will store data in an internal buffer that can be retrieved
// using stream.writable_buffer_ or stream.readable_buffer_, respectively.
//
// The amount of data potentially buffered depends on the high_water_mark_ option
// passed into the stream's constructor.
// The high_water_mark_ option specifies a total number of bytes.
//
// Data is buffered in readable_buffer_ when the implementation calls stream.push(chunk).
// If the consumer of the Stream does not call stream.read(),
// the data will sit in the internal readable_buffer_ until it is consumed.
//
// Once the total size of the internal read buffer reaches the threshold specified by high_water_mark_,
// the stream will temporarily stop reading data from the underlying resource
// until the data currently buffered can be consumed (that is,
// the stream will stop calling the internal _read() method that is used to fill the read buffer).
//
// Data is buffered in writable_buffer_ when the stream.write(chunk) method is called repeatedly.
// While the total size of the internal write buffer is below the threshold set by high_water_mark_,
// calls to stream.write() will return true.
// Once the size of the internal buffer reaches or exceeds the high_water_mark_, false will be returned.
//
// A key goal of the stream API, is to limit the buffering of data to acceptable levels
// such that sources and destinations of differing speeds will not overwhelm the available memory.
//
// Because data may be written to the socket at a faster or slower rate than data is received,
// each side should operate (and buffer) independently of the other.

class duplex_stream : public event_emitter {
public:
    ~duplex_stream() override = default;

    duplex_stream(size_t high_water_mark)
            : high_water_mark_(high_water_mark) {}

    /**
     * The stream.write() method writes some data to the stream.
     * The return value is true if the internal buffer is less than the high_water_mark_ configured
     * when the stream was created after admitting chunk.
     * If false is returned, further attempts to write data to the stream should stop
     * until the 'drain' event is emitted.
     *
     * While a stream is not draining, calls to write() will buffer chunk, and return false.
     * Once all currently buffered chunks are drained (accepted for delivery by the operating system),
     * the 'drain' event will be emitted.
     * It is recommended that once write() returns false, no more chunks be written until the 'drain' event is emitted.
     * While calling write() on a stream that is not draining is allowed,
     * stream will buffer all written chunks until maximum memory usage occurs,
     * at which point it will abort unconditionally.
     */
    virtual bool write(const std::string &data) = 0;

    /**
     * The stream.read() method pulls some data out of the internal buffer and returns it.
     * It returns the number of bytes read (0 <= n <= buffer_len)
     *
     * Only after readable.read() returns null, 'readable' will be emitted.
     * Calling stream.read() after the 'end' event has been emitted will return 0.
     * No runtime error will be raised.
     */
    virtual std::string read() = 0;

    /**
     * Calling the stream.end() method signals that no more data will be written to the stream.
     * all data in the writable_buffer_  flushed to the underlying system
     * Calling the stream.write() method after calling stream.end() will raise an error.
     */
    virtual void end() = 0;

    /**
     * Destroy the stream.
     * emit a 'close' event.
     * After this call, the stream has ended and subsequent calls to write() or end() will result in an ERR_STREAM_DESTROYED error.
     * After this call, the readable stream will release any internal resources and subsequent calls to push() will be ignored.
     * This is a destructive and immediate way to destroy a stream.
     * Previous calls to write() may not have drained, and may trigger an ERR_STREAM_DESTROYED error.
     * Use end() instead of destroy if data should flush before close,
     * or wait for the 'drain' event before destroying the stream.
     *
     * Implementors should not override this method, but instead implement writable._destroy().
     */
    void destroy() {
        _destroy();
        //todo.do something
    }

protected:
    virtual void _destroy() = 0;

protected:
    size_t high_water_mark_ = 0;
};

// Events
//
// 'close' ->
// The 'close' event is emitted when the stream and any of its underlying resources
// (a socket, for example) have been closed.
// The event indicates that no more events will be emitted, and no further computation will occur.
//
// 'drain' ->
// If a call to stream.write(chunk) returns false,
// the 'drain' event will be emitted when it is appropriate to resume writing data to the stream.
//
// 'error' ->
// The 'error' event is emitted if an error occurred while writing data or
// when a stream implementation attempts to push an invalid chunk of data.
//
// The listener callback is passed a single Error argument when called.
// The stream is auto closed/destroy when the 'error' event is emitted.
// After 'error', no further events other than 'close' should be emitted (including 'error' events).
//
// 'finish' ->
// The 'finish' event is emitted after the stream.end() method has been called,
// and all data has been flushed to the underlying system.
//
// 'readable' ->
// The 'readable' event is emitted when there is data available to be read from the stream.
// In some cases, attaching a listener for the 'readable' event will cause some amount of data to be read into an internal buffer.
// The 'readable' event will also be emitted once the end of the stream data has been reached but before the 'end' event is emitted.
//
// Effectively, the 'readable' event indicates that the stream has new information:
// either new data is available or the end of the stream has been reached.
// In the former case, stream.read() will return the available data.
// In the latter case, stream.read() will return null.

#endif //SCUTTLEBUTT_DUPLEX_STREAM_H
