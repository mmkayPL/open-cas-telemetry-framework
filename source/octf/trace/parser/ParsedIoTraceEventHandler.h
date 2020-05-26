/*
 * Copyright(c) 2012-2020 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef SOURCE_OCTF_TRACE_PARSER_PARSEDIOTRACEEVENTHANDLER_H
#define SOURCE_OCTF_TRACE_PARSER_PARSEDIOTRACEEVENTHANDLER_H

#include <map>
#include <memory>
#include <queue>
#include <set>
#include <octf/fs/FileId.h>
#include <octf/fs/IFileSystemViewer.h>
#include <octf/interface/IIoTraceParser.h>
#include <octf/proto/parsedTrace.pb.h>
#include <octf/proto/trace.pb.h>
#include <octf/trace/parser/TraceEventHandler.h>

namespace octf {

/**
 * This is IO trace event handler of parsed IO
 *
 * The parsed IO is created from multiple events (especially IO request and IO
 * completion). It contains basic IO information (LBA, length, etc). It is
 * supplemented by related information like filesystem one. In addition it
 * provides post parse information (latency, queue depth, etc...).
 *
 * @note The order of handled IO respect the IOs queuing order
 */
class ParsedIoTraceEventHandler
        : public TraceEventHandler<proto::trace::Event> {
public:
    ParsedIoTraceEventHandler(const std::string &tracePath);
    virtual ~ParsedIoTraceEventHandler();

    /**
     * @brief Handles parsed IO
     *
     * @param IO Parsed IO to be handle
     */
    virtual void handleIO(const proto::trace::ParsedEvent &io) = 0;

    void processEvents() override {
        TraceEventHandler<proto::trace::Event>::processEvents();
        m_childParser->flushEvents();
    }

    /**
     * @return Sum of all devices sizes in sectors
     */
    uint64_t getDevicesSize() const;

    /**
     * @brief Handles device description trace event
     *
     * @param devDesc Device description trace event
     */
    virtual void handleDeviceDescription(
            const proto::trace::EventDeviceDescription &devDesc) {
        (void) devDesc;
    }

protected:
    /**
     * Gets filesystem viewer interface
     *
     * This interface is used to inspect and view filesystem on the basis
     * of captured IO traces.
     *
     * @param deviceID
     *
     * @return
     */
    IFileSystemViewer *getFileSystemViewer(uint64_t partitionID);

    /**
     * @brief Skip IO's outside of this defined subrange
     * @param start LBA of subrange start
     * @param end LBA of subrange end
     *
     * @note any IO's which overlap with this range will also be included
     * @note when subrange is set, queue depth may be meaningless - use this
     *  only when queue depth is not considered - also that's why it's protected
     */
    void setExclusiveSubrange(uint64_t start, uint64_t end);

private:
    bool compareEvents(const proto::trace::Event *a,
                       const proto::trace::Event *b) override {
        return a->header().sid() < b->header().sid();
    }

    void handleEvent(std::shared_ptr<proto::trace::Event> traceEvent) override;

    virtual std::shared_ptr<proto::trace::Event> getEventMessagePrototype()
            override;

private:
    std::shared_ptr<IIoTraceParser> m_childParser;
};

}  // namespace octf

#endif  // SOURCE_OCTF_TRACE_PARSER_PARSEDIOTRACEEVENTHANDLER_H
