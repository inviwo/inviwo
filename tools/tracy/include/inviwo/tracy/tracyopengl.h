#pragma once

// This file is a modified copy of TracyOpenGL.hpp from tracy
// We have renamed all macros to use SCREAMING_SNAKE_CASE which is the inviwo macro style
// We have also prepended TRACY_ to all macro that did not already have that prefix

// clang-format off
#if !defined TRACY_ENABLE || defined __APPLE__ 

#define TRACY_GPU_CONTEXT
#define TRACY_GPU_CONTEXT_NAME(x,y)
#define TRACY_GPU_NAMED_ZONE(x,y,z)
#define TRACY_GPU_NAMED_ZONE_C(x,y,z,w)
#define TRACY_GPU_ZONE(x)
#define TRACY_GPU_ZONE_C(x,y)
#define TRACY_GPU_ZONE_TRANSIENT(x,y,z)
#define TRACY_GPU_COLLECT

#define TRACY_GPU_NAMED_ZONE_S(x,y,z,w)
#define TRACY_GPU_NAMED_ZONE_C_S(x,y,z,w,a)
#define TRACY_GPU_ZONE_S(x,y)
#define TRACY_GPU_ZONE_C_S(x,y,z)
#define TRACY_GPU_ZONE_TRANSIENT_S(x,y,z,w)

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
#include "client/TracyProfiler.hpp"
#include "client/TracyCallstack.hpp"
#include "common/TracyAlign.hpp"
#include "common/TracyAlloc.hpp"

#if !defined GL_TIMESTAMP && defined GL_TIMESTAMP_EXT
#  define GL_TIMESTAMP GL_TIMESTAMP_EXT
#  define GL_QUERY_COUNTER_BITS GL_QUERY_COUNTER_BITS_EXT
#  define glGetQueryObjectiv glGetQueryObjectivEXT
#  define glGetQueryObjectui64v glGetQueryObjectui64vEXT
#  define glQueryCounter glQueryCounterEXT
#endif

#define TRACY_GPU_CONTEXT tracy::GetGpuCtx().ptr = (tracy::GpuCtx*)tracy::tracy_malloc( sizeof( tracy::GpuCtx ) ); new(tracy::GetGpuCtx().ptr) tracy::GpuCtx;
#define TRACY_GPU_CONTEXT_NAME( name, size ) tracy::GetGpuCtx().ptr->Name( name, size );
#if defined TRACY_HAS_CALLSTACK && defined TRACY_CALLSTACK
#  define TRACY_GPU_NAMED_ZONE( varname, name, active ) static constexpr tracy::SourceLocationData TracyConcat(__tracy_gpu_source_location,__LINE__) { name, __FUNCTION__,  __FILE__, (uint32_t)__LINE__, 0 }; tracy::GpuCtxScope varname( &TracyConcat(__tracy_gpu_source_location,__LINE__), TRACY_CALLSTACK, active );
#  define TRACY_GPU_NAMED_ZONE_C( varname, name, color, active ) static constexpr tracy::SourceLocationData TracyConcat(__tracy_gpu_source_location,__LINE__) { name, __FUNCTION__,  __FILE__, (uint32_t)__LINE__, color }; tracy::GpuCtxScope varname( &TracyConcat(__tracy_gpu_source_location,__LINE__), TRACY_CALLSTACK, active );
#  define TRACY_GPU_ZONE( name ) TRACY_GPU_NAMED_ZONE_S( ___tracy_gpu_zone, name, TRACY_CALLSTACK, true )
#  define TRACY_GPU_ZONE_C( name, color ) TRACY_GPU_NAMED_ZONE_C_S( ___tracy_gpu_zone, name, color, TRACY_CALLSTACK, true )
#  define TRACY_GPU_ZONE_TRANSIENT( varname, name, active ) tracy::GpuCtxScope varname( __LINE__, __FILE__, strlen( __FILE__ ), __FUNCTION__, strlen( __FUNCTION__ ), name, strlen( name ), TRACY_CALLSTACK, active );
#else
#  define TRACY_GPU_NAMED_ZONE( varname, name, active ) static constexpr tracy::SourceLocationData TracyConcat(__tracy_gpu_source_location,__LINE__) { name, __FUNCTION__,  __FILE__, (uint32_t)__LINE__, 0 }; tracy::GpuCtxScope varname( &TracyConcat(__tracy_gpu_source_location,__LINE__), active );
#  define TRACY_GPU_NAMED_ZONE_C( varname, name, color, active ) static constexpr tracy::SourceLocationData TracyConcat(__tracy_gpu_source_location,__LINE__) { name, __FUNCTION__,  __FILE__, (uint32_t)__LINE__, color }; tracy::GpuCtxScope varname( &TracyConcat(__tracy_gpu_source_location,__LINE__), active );
#  define TRACY_GPU_ZONE( name ) TRACY_GPU_NAMED_ZONE( ___tracy_gpu_zone, name, true )
#  define TRACY_GPU_ZONE_C( name, color ) TRACY_GPU_NAMED_ZONE_C( ___tracy_gpu_zone, name, color, true )
#  define TRACY_GPU_ZONE_TRANSIENT( varname, name, active ) tracy::GpuCtxScope varname( __LINE__, __FILE__, strlen( __FILE__ ), __FUNCTION__, strlen( __FUNCTION__ ), name, strlen( name ), active );
#endif
#define TRACY_GPU_COLLECT tracy::GetGpuCtx().ptr->Collect();

#ifdef TRACY_HAS_CALLSTACK
#  define TRACY_GPU_NAMED_ZONE_S( varname, name, depth, active ) static constexpr tracy::SourceLocationData TracyConcat(__tracy_gpu_source_location,__LINE__) { name, __FUNCTION__,  __FILE__, (uint32_t)__LINE__, 0 }; tracy::GpuCtxScope varname( &TracyConcat(__tracy_gpu_source_location,__LINE__), depth, active );
#  define TRACY_GPU_NAMED_ZONE_C_S( varname, name, color, depth, active ) static constexpr tracy::SourceLocationData TracyConcat(__tracy_gpu_source_location,__LINE__) { name, __FUNCTION__,  __FILE__, (uint32_t)__LINE__, color }; tracy::GpuCtxScope varname( &TracyConcat(__tracy_gpu_source_location,__LINE__), depth, active );
#  define TRACY_GPU_ZONE_S( name, depth ) TRACY_GPU_NAMED_ZONE_S( ___tracy_gpu_zone, name, depth, true )
#  define TRACY_GPU_ZONE_C_S( name, color, depth ) TRACY_GPU_NAMED_ZONE_C_S( ___tracy_gpu_zone, name, color, depth, true )
#  define TRACY_GPU_ZONE_TRANSIENT_S( varname, name, depth, active ) tracy::GpuCtxScope varname( __LINE__, __FILE__, strlen( __FILE__ ), __FUNCTION__, strlen( __FUNCTION__ ), name, strlen( name ), depth, active );
#else
#  define TRACY_GPU_NAMED_ZONE_S( varname, name, depth, active ) TRACY_GPU_NAMED_ZONE( varname, name, active )
#  define TRACY_GPU_NAMED_ZONE_C_S( varname, name, color, depth, active ) TRACY_GPU_NAMED_ZONE_C( varname, name, color, active )
#  define TRACY_GPU_ZONE_S( name, depth ) TRACY_GPU_ZONE( name )
#  define TRACY_GPU_ZONE_C_S( name, color, depth ) TRACY_GPU_ZONE_C( name, color )
#  define TRACY_GPU_ZONE_TRANSIENT_S( varname, name, depth, active ) TracyGpuZoneTransient( varname, name, active )
#endif

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

    void Name( const char* name, uint16_t len )
    {
        auto ptr = (char*)tracy_malloc( len );
        memcpy( ptr, name, len );

        TracyLfqPrepare( QueueType::GpuContextName );
        MemWrite( &item->gpuContextNameFat.context, m_context );
        MemWrite( &item->gpuContextNameFat.ptr, (uint64_t)ptr );
        MemWrite( &item->gpuContextNameFat.size, len );
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
        memset( &item->gpuZoneBegin.thread, 0, sizeof( item->gpuZoneBegin.thread ) );
        MemWrite( &item->gpuZoneBegin.queryId, uint16_t( queryId ) );
        MemWrite( &item->gpuZoneBegin.context, GetGpuCtx().ptr->GetId() );
        MemWrite( &item->gpuZoneBegin.srcloc, (uint64_t)srcloc );
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

#ifdef TRACY_FIBERS
        TracyLfqPrepare( QueueType::GpuZoneBegin );
        memset( &item->gpuZoneBegin.thread, 0, sizeof( item->gpuZoneBegin.thread ) );
#else
        GetProfiler().SendCallstack( depth );
        TracyLfqPrepare( QueueType::GpuZoneBeginCallstack );
        MemWrite( &item->gpuZoneBegin.thread, GetThreadHandle() );
#endif
        MemWrite( &item->gpuZoneBegin.cpuTime, Profiler::GetTime() );
        MemWrite( &item->gpuZoneBegin.queryId, uint16_t( queryId ) );
        MemWrite( &item->gpuZoneBegin.context, GetGpuCtx().ptr->GetId() );
        MemWrite( &item->gpuZoneBegin.srcloc, (uint64_t)srcloc );
        TracyLfqCommit;
    }

    tracy_force_inline GpuCtxScope( uint32_t line, const char* source, size_t sourceSz, const char* function, size_t functionSz, const char* name, size_t nameSz, bool is_active )
#ifdef TRACY_ON_DEMAND
        : m_active( is_active && GetProfiler().IsConnected() )
#else
        : m_active( is_active )
#endif
    {
        if( !m_active ) return;

        const auto queryId = GetGpuCtx().ptr->NextQueryId();
        glQueryCounter( GetGpuCtx().ptr->TranslateOpenGlQueryId( queryId ), GL_TIMESTAMP );

        TracyLfqPrepare( QueueType::GpuZoneBeginAllocSrcLoc );
        const auto srcloc = Profiler::AllocSourceLocation( line, source, sourceSz, function, functionSz, name, nameSz );
        MemWrite( &item->gpuZoneBegin.cpuTime, Profiler::GetTime() );
        memset( &item->gpuZoneBegin.thread, 0, sizeof( item->gpuZoneBegin.thread ) );
        MemWrite( &item->gpuZoneBegin.queryId, uint16_t( queryId ) );
        MemWrite( &item->gpuZoneBegin.context, GetGpuCtx().ptr->GetId() );
        MemWrite( &item->gpuZoneBegin.srcloc, (uint64_t)srcloc );
        TracyLfqCommit;
    }

    tracy_force_inline GpuCtxScope( uint32_t line, const char* source, size_t sourceSz, const char* function, size_t functionSz, const char* name, size_t nameSz, int depth, bool is_active )
#ifdef TRACY_ON_DEMAND
        : m_active( is_active && GetProfiler().IsConnected() )
#else
        : m_active( is_active )
#endif
    {
        if( !m_active ) return;

        const auto queryId = GetGpuCtx().ptr->NextQueryId();
        glQueryCounter( GetGpuCtx().ptr->TranslateOpenGlQueryId( queryId ), GL_TIMESTAMP );

#ifdef TRACY_FIBERS
        TracyLfqPrepare( QueueType::GpuZoneBeginAllocSrcLoc );
        memset( &item->gpuZoneBegin.thread, 0, sizeof( item->gpuZoneBegin.thread ) );
#else
        GetProfiler().SendCallstack( depth );
        TracyLfqPrepare( QueueType::GpuZoneBeginAllocSrcLocCallstack );
        MemWrite( &item->gpuZoneBegin.thread, GetThreadHandle() );
#endif
        const auto srcloc = Profiler::AllocSourceLocation( line, source, sourceSz, function, functionSz, name, nameSz );
        MemWrite( &item->gpuZoneBegin.cpuTime, Profiler::GetTime() );
        MemWrite( &item->gpuZoneBegin.queryId, uint16_t( queryId ) );
        MemWrite( &item->gpuZoneBegin.context, GetGpuCtx().ptr->GetId() );
        MemWrite( &item->gpuZoneBegin.srcloc, (uint64_t)srcloc );
        TracyLfqCommit;
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
