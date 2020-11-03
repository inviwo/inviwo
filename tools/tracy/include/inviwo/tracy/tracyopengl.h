#pragma once

#if !defined GL_TIMESTAMP && !defined GL_TIMESTAMP_EXT
#  error "You must include OpenGL 3.2 headers before including TracyOpenGL.hpp"
#endif

#if !defined TRACY_ENABLE || defined __APPLE__

#define TRACY_GPU_CONTEXT
#define TRACY_GPU_NAMED_ZONE(x,y,z)
#define TRACY_GPU_NAMED_ZONE_C(x,y,z,w)
#define TRACY_GPU_ZONE(x)
#define TRACY_GPU_ZONE_C(x,y)
#define TRACY_GPU_COLLECT

#define TRACY_GPU_NAMED_ZONE_S(x,y,z,w)
#define TRACY_GPU_NAMED_ZONE_CS(x,y,z,w,a)
#define TRACY_GPU_ZONE_S(x,y)
#define TRACY_GPU_ZONE_CS(x,y,z)

namespace tracy
{
struct SourceLocationData;
class GpuCtxScope
{
public:
    GpuCtxScope( const SourceLocationData*, bool ) {}
    GpuCtxScope( const SourceLocationData*, int, bool ) {}
};
}

#else

#include <atomic>
#include <assert.h>
#include <stdlib.h>

#include "tracy.h"
#include <tracy/client/TracyProfiler.hpp>
#include <tracy/client/TracyCallstack.hpp>
#include <tracy/common/TracyAlign.hpp>
#include <tracy/common/TracyAlloc.hpp>

#if !defined GL_TIMESTAMP && defined GL_TIMESTAMP_EXT
#  define GL_TIMESTAMP GL_TIMESTAMP_EXT
#  define GL_QUERY_COUNTER_BITS GL_QUERY_COUNTER_BITS_EXT
#  define glGetQueryObjectiv glGetQueryObjectivEXT
#  define glGetQueryObjectui64v glGetQueryObjectui64vEXT
#  define glQueryCounter glQueryCounterEXT
#endif

#define TRACY_GPU_CONTEXT tracy::InitRPMallocThread(); tracy::GetGpuCtx().ptr = (tracy::GpuCtx*)tracy::tracy_malloc( sizeof( tracy::GpuCtx ) ); new(tracy::GetGpuCtx().ptr) tracy::GpuCtx;
#define TRACY_GPU_NAMED_ZONE( varname, name, active ) static constexpr tracy::SourceLocationData TracyConcat(__tracy_gpu_source_location,__LINE__) { name, __FUNCTION__,  __FILE__, (uint32_t)__LINE__, 0 }; tracy::GpuCtxScope varname( &TracyConcat(__tracy_gpu_source_location,__LINE__), active );
#define TRACY_GPU_NAMED_ZONE_C( varname, name, color, active ) static constexpr tracy::SourceLocationData TracyConcat(__tracy_gpu_source_location,__LINE__) { name, __FUNCTION__,  __FILE__, (uint32_t)__LINE__, color }; tracy::GpuCtxScope varname( &TracyConcat(__tracy_gpu_source_location,__LINE__), active );
#define TRACY_GPU_ZONE( name ) TRACY_GPU_NAMED_ZONE( ___tracy_gpu_zone, name, true )
#define TRACY_GPU_ZONE_C( name, color ) TRACY_GPU_NAMED_ZONE_C( ___tracy_gpu_zone, name, color, true )
#define TRACY_GPU_COLLECT tracy::GetGpuCtx().ptr->Collect();
#define TRACY_GPU_NAMED_ZONE_S( varname, name, depth, active ) static constexpr tracy::SourceLocationData TracyConcat(__tracy_gpu_source_location,__LINE__) { name, __FUNCTION__,  __FILE__, (uint32_t)__LINE__, 0 }; tracy::GpuCtxScope varname( &TracyConcat(__tracy_gpu_source_location,__LINE__), depth, active );
#define TRACY_GPU_NAMED_ZONE_CS( varname, name, color, depth, active ) static constexpr tracy::SourceLocationData TracyConcat(__tracy_gpu_source_location,__LINE__) { name, __FUNCTION__,  __FILE__, (uint32_t)__LINE__, color }; tracy::GpuCtxScope varname( &TracyConcat(__tracy_gpu_source_location,__LINE__), depth, active );
#define TRACY_GPU_ZONE_S( name, depth ) TRACY_GPU_NAMED_ZONE_S( ___tracy_gpu_zone, name, depth, true )
#define TRACY_GPU_ZONE_CS( name, color, depth ) TRACY_GPU_NAMED_ZONE_CS( ___tracy_gpu_zone, name, color, depth, true )

namespace tracy
{

class GpuCtx
{
    friend class GpuCtxScope;

    enum { QueryCount = 64 * 1024 };

public:
    GpuCtx()
        : m_context( GetGpuCtxCounter().fetch_add( 1, std::memory_order_relaxed ) )
        , m_head( 0 )
        , m_tail( 0 )
    {
        assert( m_context != 255 );

        glGenQueries( QueryCount, m_query );

        int64_t tgpu;
        glGetInteger64v( GL_TIMESTAMP, &tgpu );
        int64_t tcpu = Profiler::GetTime();

        GLint bits;
        glGetQueryiv( GL_TIMESTAMP, GL_QUERY_COUNTER_BITS, &bits );

        const float period = 1.f;
        const auto thread = GetThreadHandle();
        TracyLfqPrepare( QueueType::GpuNewContext );
        MemWrite( &item->gpuNewContext.cpuTime, tcpu );
        MemWrite( &item->gpuNewContext.gpuTime, tgpu );
        MemWrite( &item->gpuNewContext.thread, thread );
        MemWrite( &item->gpuNewContext.period, period );
        MemWrite( &item->gpuNewContext.context, m_context );
        MemWrite( &item->gpuNewContext.flags, uint8_t( 0 ) );
        MemWrite( &item->gpuNewContext.type, GpuContextType::OpenGl );

#ifdef TRACY_ON_DEMAND
        GetProfiler().DeferItem( *item );
#endif

        TracyLfqCommit;
    }

    void Collect()
    {
        TRACY_ZONE_SCOPED_C( Color::Red4 );

        if( m_tail == m_head ) return;

#ifdef TRACY_ON_DEMAND
        if( !GetProfiler().IsConnected() )
        {
            m_head = m_tail = 0;
            return;
        }
#endif

        while( m_tail != m_head )
        {
            GLint available;
            glGetQueryObjectiv( m_query[m_tail], GL_QUERY_RESULT_AVAILABLE, &available );
            if( !available ) return;

            uint64_t time;
            glGetQueryObjectui64v( m_query[m_tail], GL_QUERY_RESULT, &time );

            TracyLfqPrepare( QueueType::GpuTime );
            MemWrite( &item->gpuTime.gpuTime, (int64_t)time );
            MemWrite( &item->gpuTime.queryId, (uint16_t)m_tail );
            MemWrite( &item->gpuTime.context, m_context );
            TracyLfqCommit;

            m_tail = ( m_tail + 1 ) % QueryCount;
        }
    }

private:
    tracy_force_inline unsigned int NextQueryId()
    {
        const auto id = m_head;
        m_head = ( m_head + 1 ) % QueryCount;
        assert( m_head != m_tail );
        return id;
    }

    tracy_force_inline unsigned int TranslateOpenGlQueryId( unsigned int id )
    {
        return m_query[id];
    }

    tracy_force_inline uint8_t GetId() const
    {
        return m_context;
    }

    unsigned int m_query[QueryCount];
    uint8_t m_context;

    unsigned int m_head;
    unsigned int m_tail;
};

class GpuCtxScope
{
public:
    tracy_force_inline GpuCtxScope( const SourceLocationData* srcloc, bool is_active )
#ifdef TRACY_ON_DEMAND
        : m_active( is_active && GetProfiler().IsConnected() )
#else
        : m_active( is_active )
#endif
    {
        if( !m_active ) return;

        const auto queryId = GetGpuCtx().ptr->NextQueryId();
        glQueryCounter( GetGpuCtx().ptr->TranslateOpenGlQueryId( queryId ), GL_TIMESTAMP );

        TracyLfqPrepare( QueueType::GpuZoneBegin );
        MemWrite( &item->gpuZoneBegin.cpuTime, Profiler::GetTime() );
        MemWrite( &item->gpuZoneBegin.srcloc, (uint64_t)srcloc );
        memset( &item->gpuZoneBegin.thread, 0, sizeof( item->gpuZoneBegin.thread ) );
        MemWrite( &item->gpuZoneBegin.queryId, uint16_t( queryId ) );
        MemWrite( &item->gpuZoneBegin.context, GetGpuCtx().ptr->GetId() );
        TracyLfqCommit;
    }

    tracy_force_inline GpuCtxScope( const SourceLocationData* srcloc, int depth, bool is_active )
#ifdef TRACY_ON_DEMAND
        : m_active( is_active && GetProfiler().IsConnected() )
#else
        : m_active( is_active )
#endif
    {
        if( !m_active ) return;

        const auto queryId = GetGpuCtx().ptr->NextQueryId();
        glQueryCounter( GetGpuCtx().ptr->TranslateOpenGlQueryId( queryId ), GL_TIMESTAMP );

        const auto thread = GetThreadHandle();
        TracyLfqPrepare( QueueType::GpuZoneBeginCallstack );
        MemWrite( &item->gpuZoneBegin.cpuTime, Profiler::GetTime() );
        MemWrite( &item->gpuZoneBegin.srcloc, (uint64_t)srcloc );
        MemWrite( &item->gpuZoneBegin.thread, thread );
        MemWrite( &item->gpuZoneBegin.queryId, uint16_t( queryId ) );
        MemWrite( &item->gpuZoneBegin.context, GetGpuCtx().ptr->GetId() );
        TracyLfqCommit;

        GetProfiler().SendCallstack( depth );
    }

    tracy_force_inline ~GpuCtxScope()
    {
        if( !m_active ) return;

        const auto queryId = GetGpuCtx().ptr->NextQueryId();
        glQueryCounter( GetGpuCtx().ptr->TranslateOpenGlQueryId( queryId ), GL_TIMESTAMP );

        TracyLfqPrepare( QueueType::GpuZoneEnd );
        MemWrite( &item->gpuZoneEnd.cpuTime, Profiler::GetTime() );
        memset( &item->gpuZoneEnd.thread, 0, sizeof( item->gpuZoneEnd.thread ) );
        MemWrite( &item->gpuZoneEnd.queryId, uint16_t( queryId ) );
        MemWrite( &item->gpuZoneEnd.context, GetGpuCtx().ptr->GetId() );
        TracyLfqCommit;
    }

private:
    const bool m_active;
};

}

#endif
