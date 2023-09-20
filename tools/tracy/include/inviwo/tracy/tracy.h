#pragma once

// This file is a modified copy of Tracy.hpp from tracy
// We have renamed all macros to use SCREAMING_SNAKE_CASE which is the inviwo macro style
// We have also prepended TRACY_ to all macro that did not already have that prefix
// We have TRACY_HAS_CALLSTACK as defined and TRACY_CALLSTACK as undefined

// clang-format off

#ifndef TRACY_ENABLE

#define TRACY_ZONE_NAMED(x,y)
#define TRACY_ZONE_NAMED_N(x,y,z)
#define TRACY_ZONE_NAMED_C(x,y,z)
#define TRACY_ZONE_NAMED_N_C(x,y,z,w)

#define TRACY_ZONE_TRANSIENT(x,y)
#define TRACY_ZONE_TRANSIENT_N(x,y,z)

#define TRACY_ZONE_SCOPED
#define TRACY_ZONE_SCOPED_N(x)
#define TRACY_ZONE_SCOPED_C(x)
#define TRACY_ZONE_SCOPED_NC(x,y)

#define TRACY_ZONE_TEXT(x,y)
#define TRACY_ZONE_TEXT_V(x,y,z)
#define TRACY_ZONE_NAME(x,y)
#define TRACY_ZONE_NAME_V(x,y,z)
#define TRACY_ZONE_COLOR(x)
#define TRACY_ZONE_COLOR_V(x,y)
#define TRACY_ZONE_VALUE(x)
#define TRACY_ZONE_VALUE_V(x,y)
#define TRACY_ZONE_IS_ACTIVE false
#define TRACY_ZONE_IS_ACTIVE_V(x) false

#define TRACY_FRAME_MARK
#define TRACY_FRAME_MARK_NAMED(x)
#define TRACY_FRAME_MARK_START(x)
#define TRACY_FRAME_MARK_END(x)

#define TRACY_FRAME_IMAGE(x,y,z,w,a)

#define TRACY_LOCKABLE( type, varname ) type varname;
#define TRACY_LOCKABLE_N( type, varname, desc ) type varname;
#define TRACY_SHARED_LOCKABLE( type, varname ) type varname;
#define TRACY_SHARED_LOCKABLE_N( type, varname, desc ) type varname;
#define TRACY_LOCKABLE_BASE( type ) type
#define TRACY_SHARED_LOCKABLE_BASE( type ) type
#define TRACY_LOCK_MARK(x) (void)x;
#define TRACY_LOCKABLE_NAME(x,y,z);

#define TRACY_PLOT(x,y)
#define TRACY_PLOT_CONFIG(x,y)

#define TRACY_MESSAGE(x,y)
#define TRACY_MESSAGE_L(x)
#define TRACY_MESSAGE_C(x,y,z)
#define TRACY_MESSAGE_L_C(x,y)
#define TRACY_APP_INFO(x,y)

#define TRACY_ALLOC(x,y)
#define TRACY_FREE(x)
#define TRACY_SECURE_ALLOC(x,y)
#define TRACY_SECURE_FREE(x)

#define TRACY_ALLOC_N(x,y,z)
#define TRACY_FREE_N(x,y)
#define TRACY_SECURE_ALLOC_N(x,y,z)
#define TRACY_SECURE_FREE_N(x,y)

#define TRACY_ZONE_NAMED_S(x,y,z)
#define TRACY_ZONE_NAMED_NS(x,y,z,w)
#define TRACY_ZONE_NAMED_CS(x,y,z,w)
#define TRACY_ZONE_NAMED_NCS(x,y,z,w,a)

#define ZTRACY_ONE_TRANSIENT_S(x,y,z)
#define TRACY_ZONE_TRANSIENT_N_S(x,y,z,w)

#define TRACY_ZONE_SCOPED_S(x)
#define TRACY_ZONE_SCOPED_NS(x,y)
#define TRACY_ZONE_SCOPED_CS(x,y)
#define TRACY_ZONE_SCOPED_NCS(x,y,z)

#define TRACY_ALLOC_S(x,y,z)
#define TRACY_FREE_S(x,y)
#define TRACY_SECURE_ALLOC_S(x,y,z)
#define TRACY_SECURE_FREE_S(x,y)

#define TRACY_ALLOC_NS(x,y,z,w)
#define TRACY_FREE_NS(x,y,z)
#define TRACY_SECURE_ALLOC_NS(x,y,z,w)
#define TRACY_SECURE_FREE_NS(x,y,z)

#define TRACY_MESSAGE_S(x,y,z)
#define TRACY_MESSAGE_LS(x,y)
#define TRACY_MESSAGE_CS(x,y,z,w)
#define TRACY_MESSAGE_LCS(x,y,z)

#define TRACY_PARAMETER_REGISTER(x)
#define TRACY_PARAMETER_SETUP(x,y,z,w)
#define TRACY_IS_CONNECTED false

#define TRACY_FIBER_ENTER(x)
#define TRACY_FIBER_LEAVE

#else

#include <string.h>

#include "common/TracyColor.hpp"
#include "common/TracySystem.hpp"

#include "client/TracyLock.hpp"
#include "client/TracyProfiler.hpp"
#include "client/TracyScoped.hpp"

#if defined TRACY_HAS_CALLSTACK && defined TRACY_CALLSTACK
#  define TRACY_ZONE_NAMED( varname, active ) static constexpr tracy::SourceLocationData TracyConcat(__tracy_source_location,__LINE__) { nullptr, __FUNCTION__,  __FILE__, (uint32_t)__LINE__, 0 }; tracy::ScopedZone varname( &TracyConcat(__tracy_source_location,__LINE__), TRACY_CALLSTACK, active );
#  define TRACY_ZONE_NAMED_N( varname, name, active ) static constexpr tracy::SourceLocationData TracyConcat(__tracy_source_location,__LINE__) { name, __FUNCTION__,  __FILE__, (uint32_t)__LINE__, 0 }; tracy::ScopedZone varname( &TracyConcat(__tracy_source_location,__LINE__), TRACY_CALLSTACK, active );
#  define TRACY_ZONE_NAMED_C( varname, color, active ) static constexpr tracy::SourceLocationData TracyConcat(__tracy_source_location,__LINE__) { nullptr, __FUNCTION__,  __FILE__, (uint32_t)__LINE__, color }; tracy::ScopedZone varname( &TracyConcat(__tracy_source_location,__LINE__), TRACY_CALLSTACK, active );
#  define TRACY_ZONE_NAMED_NC( varname, name, color, active ) static constexpr tracy::SourceLocationData TracyConcat(__tracy_source_location,__LINE__) { name, __FUNCTION__,  __FILE__, (uint32_t)__LINE__, color }; tracy::ScopedZone varname( &TracyConcat(__tracy_source_location,__LINE__), TRACY_CALLSTACK, active );

#  define TRACY_ZONE_TRANSIENT( varname, active ) tracy::ScopedZone varname( __LINE__, __FILE__, strlen( __FILE__ ), __FUNCTION__, strlen( __FUNCTION__ ), nullptr, 0, TRACY_CALLSTACK, active );
#  define TRACY_ZONE_TRANSIENT_N( varname, name, active ) tracy::ScopedZone varname( __LINE__, __FILE__, strlen( __FILE__ ), __FUNCTION__, strlen( __FUNCTION__ ), name, strlen( name ), TRACY_CALLSTACK, active );
#else
#  define TRACY_ZONE_NAMED( varname, active ) static constexpr tracy::SourceLocationData TracyConcat(__tracy_source_location,__LINE__) { nullptr, __FUNCTION__,  __FILE__, (uint32_t)__LINE__, 0 }; tracy::ScopedZone varname( &TracyConcat(__tracy_source_location,__LINE__), active );
#  define TRACY_ZONE_NAMED_N( varname, name, active ) static constexpr tracy::SourceLocationData TracyConcat(__tracy_source_location,__LINE__) { name, __FUNCTION__,  __FILE__, (uint32_t)__LINE__, 0 }; tracy::ScopedZone varname( &TracyConcat(__tracy_source_location,__LINE__), active );
#  define TRACY_ZONE_NAMED_C( varname, color, active ) static constexpr tracy::SourceLocationData TracyConcat(__tracy_source_location,__LINE__) { nullptr, __FUNCTION__,  __FILE__, (uint32_t)__LINE__, color }; tracy::ScopedZone varname( &TracyConcat(__tracy_source_location,__LINE__), active );
#  define TRACY_ZONE_NAMED_NC( varname, name, color, active ) static constexpr tracy::SourceLocationData TracyConcat(__tracy_source_location,__LINE__) { name, __FUNCTION__,  __FILE__, (uint32_t)__LINE__, color }; tracy::ScopedZone varname( &TracyConcat(__tracy_source_location,__LINE__), active );

#  define TRACY_ZONE_TRANSIENT( varname, active ) tracy::ScopedZone varname( __LINE__, __FILE__, strlen( __FILE__ ), __FUNCTION__, strlen( __FUNCTION__ ), nullptr, 0, active );
#  define TRACY_ZONE_TRANSIENT_N( varname, name, active ) tracy::ScopedZone varname( __LINE__, __FILE__, strlen( __FILE__ ), __FUNCTION__, strlen( __FUNCTION__ ), name, strlen( name ), active );
#endif

#define TRACY_ZONE_SCOPED TRACY_ZONE_NAMED ___tracy_scoped_zone, true )
#define TRACY_ZONE_SCOPED_N( name ) TRACY_ZONE_NAMED_N( ___tracy_scoped_zone, name, true )
#define TRACY_ZONE_SCOPED_C( color ) TRACY_ZONE_NAMED_C( ___tracy_scoped_zone, color, true )
#define TRACY_ZONE_SCOPED_NC( name, color ) TRACY_ZONE_NAMED_NC( ___tracy_scoped_zone, name, color, true )

#define TRACY_ZONE_TEXT( txt, size ) ___tracy_scoped_zone.Text( txt, size );
#define TRACY_ZONE_TEXT_V( varname, txt, size ) varname.Text( txt, size );
#define TRACY_ZONE_NAME( txt, size ) ___tracy_scoped_zone.Name( txt, size );
#define TRACY_ZONE_NAME_V( varname, txt, size ) varname.Name( txt, size );
#define TRACY_ZONE_COLOR( color ) ___tracy_scoped_zone.Color( color );
#define TRACY_ZONE_COLOR_V( varname, color ) varname.Color( color );
#define TRACY_ZONE_VALUE( value ) ___tracy_scoped_zone.Value( value );
#define TRACY_ZONE_VALUE_V( varname, value ) varname.Value( value );
#define TRACY_ZONE_IS_ACTIVE ___tracy_scoped_zone.IsActive()
#define TRACY_ZONE_IS_ACTIVE_V( varname ) varname.IsActive()

#define TRACY_FRAME_MARK tracy::Profiler::SendFrameMark( nullptr );
#define TRACY_FRAME_MARK_NAMED( name ) tracy::Profiler::SendFrameMark( name );
#define TRACY_FRAME_MARK_START( name ) tracy::Profiler::SendFrameMark( name, tracy::QueueType::FrameMarkMsgStart );
#define TRACY_FRAME_MARK_END( name ) tracy::Profiler::SendFrameMark( name, tracy::QueueType::FrameMarkMsgEnd );

#define TRACY_FRAME_IMAGE( image, width, height, offset, flip ) tracy::Profiler::SendFrameImage( image, width, height, offset, flip );

#define TRACY_LOCKABLE( type, varname ) tracy::Lockable<type> varname { [] () -> const tracy::SourceLocationData* { static constexpr tracy::SourceLocationData srcloc { nullptr, #type " " #varname, __FILE__, __LINE__, 0 }; return &srcloc; }() };
#define TRACY_LOCKABLE_N( type, varname, desc ) tracy::Lockable<type> varname { [] () -> const tracy::SourceLocationData* { static constexpr tracy::SourceLocationData srcloc { nullptr, desc, __FILE__, __LINE__, 0 }; return &srcloc; }() };
#define TRACY_SHARED_LOCKABLE( type, varname ) tracy::SharedLockable<type> varname { [] () -> const tracy::SourceLocationData* { static constexpr tracy::SourceLocationData srcloc { nullptr, #type " " #varname, __FILE__, __LINE__, 0 }; return &srcloc; }() };
#define TRACY_SHARED_LOCKABLE_N( type, varname, desc ) tracy::SharedLockable<type> varname { [] () -> const tracy::SourceLocationData* { static constexpr tracy::SourceLocationData srcloc { nullptr, desc, __FILE__, __LINE__, 0 }; return &srcloc; }() };
#define TRACY_LOCKABLE_BASE( type ) tracy::Lockable<type>
#define TRACY_SHARED_LOCKABLE_BASE( type ) tracy::SharedLockable<type>
#define TRACY_LOCK_MARK( varname ) static constexpr tracy::SourceLocationData __tracy_lock_location_##varname { nullptr, __FUNCTION__,  __FILE__, (uint32_t)__LINE__, 0 }; varname.Mark( &__tracy_lock_location_##varname );
#define TRACY_LOCKABLE_NAME( varname, txt, size ) varname.CustomName( txt, size );

#define TRACY_PLOT( name, val ) tracy::Profiler::PlotData( name, val );
#define TRACY_PLOT_CONFIG( name, type ) tracy::Profiler::ConfigurePlot( name, type );

#define TRACY_APP_INFO( txt, size ) tracy::Profiler::MessageAppInfo( txt, size );

#if defined TRACY_HAS_CALLSTACK && defined TRACY_CALLSTACK
#  define TRACY_MESSAGE( txt, size ) tracy::Profiler::Message( txt, size, TRACY_CALLSTACK );
#  define TRACY_MESSAGE_L( txt ) tracy::Profiler::Message( txt, TRACY_CALLSTACK );
#  define TRACY_MESSAGE_C( txt, size, color ) tracy::Profiler::MessageColor( txt, size, color, TRACY_CALLSTACK );
#  define TRACY_MESSAGE_LC( txt, color ) tracy::Profiler::MessageColor( txt, color, TRACY_CALLSTACK );

#  define TRACY_ALLOC( ptr, size ) tracy::Profiler::MemAllocCallstack( ptr, size, TRACY_CALLSTACK, false );
#  define TRACY_FREE( ptr ) tracy::Profiler::MemFreeCallstack( ptr, TRACY_CALLSTACK, false );
#  define TRACY_SECURE_ALLOC( ptr, size ) tracy::Profiler::MemAllocCallstack( ptr, size, TRACY_CALLSTACK, true );
#  define TRACY_SECURE_FREE( ptr ) tracy::Profiler::MemFreeCallstack( ptr, TRACY_CALLSTACK, true );

#  define TRACY_ALLOC_N( ptr, size, name ) tracy::Profiler::MemAllocCallstackNamed( ptr, size, TRACY_CALLSTACK, false, name );
#  define TRACY_FREE_N( ptr, name ) tracy::Profiler::MemFreeCallstackNamed( ptr, TRACY_CALLSTACK, false, name );
#  define TRACY_SECURE_ALLOC_N( ptr, size, name ) tracy::Profiler::MemAllocCallstackNamed( ptr, size, TRACY_CALLSTACK, true, name );
#  define TRACY_SECURE_FREE_N( ptr, name ) tracy::Profiler::MemFreeCallstackNamed( ptr, TRACY_CALLSTACK, true, name );
#else
#  define TRACY_MESSAGE( txt, size ) tracy::Profiler::Message( txt, size, 0 );
#  define TRACY_MESSAGE_L( txt ) tracy::Profiler::Message( txt, 0 );
#  define TRACY_MESSAGE_C( txt, size, color ) tracy::Profiler::MessageColor( txt, size, color, 0 );
#  define TRACY_MESSAGE_LC( txt, color ) tracy::Profiler::MessageColor( txt, color, 0 );

#  define TRACY_ALLOC( ptr, size ) tracy::Profiler::MemAlloc( ptr, size, false );
#  define TRACY_FREE( ptr ) tracy::Profiler::MemFree( ptr, false );
#  define TRACY_SECURE_ALLOC( ptr, size ) tracy::Profiler::MemAlloc( ptr, size, true );
#  define TRACY_SECURE_FREE( ptr ) tracy::Profiler::MemFree( ptr, true );

#  define TRACY_ALLOC_N( ptr, size, name ) tracy::Profiler::MemAllocNamed( ptr, size, false, name );
#  define TRACY_FREE_N( ptr, name ) tracy::Profiler::MemFreeNamed( ptr, false, name );
#  define TRACY_SECURE_ALLOC_N( ptr, size, name ) tracy::Profiler::MemAllocNamed( ptr, size, true, name );
#  define TRACY_SECURE_FREE_N( ptr, name ) tracy::Profiler::MemFreeNamed( ptr, true, name );
#endif

#ifdef TRACY_HAS_CALLSTACK
#  define ZONE_NAMED_S( varname, depth, active ) static constexpr tracy::SourceLocationData TracyConcat(__tracy_source_location,__LINE__) { nullptr, __FUNCTION__,  __FILE__, (uint32_t)__LINE__, 0 }; tracy::ScopedZone varname( &TracyConcat(__tracy_source_location,__LINE__), depth, active );
#  define ZONE_NAMED_NS( varname, name, depth, active ) static constexpr tracy::SourceLocationData TracyConcat(__tracy_source_location,__LINE__) { name, __FUNCTION__,  __FILE__, (uint32_t)__LINE__, 0 }; tracy::ScopedZone varname( &TracyConcat(__tracy_source_location,__LINE__), depth, active );
#  define ZONE_NAMED_CS( varname, color, depth, active ) static constexpr tracy::SourceLocationData TracyConcat(__tracy_source_location,__LINE__) { nullptr, __FUNCTION__,  __FILE__, (uint32_t)__LINE__, color }; tracy::ScopedZone varname( &TracyConcat(__tracy_source_location,__LINE__), depth, active );
#  define ZONE_NAMED_NCS( varname, name, color, depth, active ) static constexpr tracy::SourceLocationData TracyConcat(__tracy_source_location,__LINE__) { name, __FUNCTION__,  __FILE__, (uint32_t)__LINE__, color }; tracy::ScopedZone varname( &TracyConcat(__tracy_source_location,__LINE__), depth, active );

#  define ZONE_TRANSIENT_S( varname, depth, active ) tracy::ScopedZone varname( __LINE__, __FILE__, strlen( __FILE__ ), __FUNCTION__, strlen( __FUNCTION__ ), nullptr, 0, depth, active );
#  define ZONE_TRANSIENT_NS( varname, name, depth, active ) tracy::ScopedZone varname( __LINE__, __FILE__, strlen( __FILE__ ), __FUNCTION__, strlen( __FUNCTION__ ), name, strlen( name ), depth, active );

#  define ZONE_SCOPED_S( depth ) TRACY_ZONE_NAMED_S( ___tracy_scoped_zone, depth, true )
#  define ZONE_SCOPED_NS( name, depth ) TRACY_ZONE_NAMED_NS( ___tracy_scoped_zone, name, depth, true )
#  define ZONE_SCOPED_CS( color, depth ) TRACY_ZONE_NAMED_CS( ___tracy_scoped_zone, color, depth, true )
#  define ZONE_SCOPED_NCS( name, color, depth ) TRACY_ZONE_NAMED_NCS( ___tracy_scoped_zone, name, color, depth, true )

#  define TRACY_ALLOC_S( ptr, size, depth ) tracy::Profiler::MemAllocCallstack( ptr, size, depth, false );
#  define TRACY_FREE_S( ptr, depth ) tracy::Profiler::MemFreeCallstack( ptr, depth, false );
#  define TRACY_SECURE_ALLOC_S( ptr, size, depth ) tracy::Profiler::MemAllocCallstack( ptr, size, depth, true );
#  define TRACY_SECURE_FREE_S( ptr, depth ) tracy::Profiler::MemFreeCallstack( ptr, depth, true );

#  define TRACY_ALLOC_NS( ptr, size, depth, name ) tracy::Profiler::MemAllocCallstackNamed( ptr, size, depth, false, name );
#  define TRACY_FREE_NS( ptr, depth, name ) tracy::Profiler::MemFreeCallstackNamed( ptr, depth, false, name );
#  define TRACY_SECURE_ALLOC_N_S( ptr, size, depth, name ) tracy::Profiler::MemAllocCallstackNamed( ptr, size, depth, true, name );
#  define TRACY_SECURE_FREE_N_S( ptr, depth, name ) tracy::Profiler::MemFreeCallstackNamed( ptr, depth, true, name );

#  define TRACY_MESSAGE_S( txt, size, depth ) tracy::Profiler::Message( txt, size, depth );
#  define TRACY_MESSAGE_LS( txt, depth ) tracy::Profiler::Message( txt, depth );
#  define TRACY_MESSAGE_CS( txt, size, color, depth ) tracy::Profiler::MessageColor( txt, size, color, depth );
#  define TRACY_MESSAGE_LCS( txt, color, depth ) tracy::Profiler::MessageColor( txt, color, depth );
#else
#  define TRACY_ZONE_NAMED_S( varname, depth, active ) TRACY_ZONE_NAMED varname, active )
#  define TRACY_ZONE_NAMED_NS( varname, name, depth, active ) TRACY_ZONE_NAMED_N( varname, name, active )
#  define TRACY_ZONE_NAMED_CS( varname, color, depth, active ) TRACY_ZONE_NAMED_C( varname, color, active )
#  define TRACY_ZONE_NAMED_NCS( varname, name, color, depth, active ) TRACY_ZONE_NAMED_NC( varname, name, color, active )

#  define TRACY_ZONE_TRANSIENT_S( varname, depth, active ) TRACY_ZONE_TRANSIENT( varname, active )
#  define TRACY_ZONE_TRANSIENT_N_S( varname, name, depth, active ) TRACY_ZONE_TRANSIENT_N( varname, name, active )

#  define TRACY_ZONE_SCOPED_S( depth ) TRACY_ZONE_SCOPED
#  define TRACY_ZONE_SCOPED_NS( name, depth ) TRACY_ZONE_SCOPED_N( name )
#  define TRACY_ZONE_SCOPED_CS( color, depth ) TRACY_ZONE_SCOPED_C( colorTRACY_ 
#  define TRACY_ZONE_SCOPED_NCS( name, color, depth ) TRACY_ZONE_SCOPED_NC( name, color )

#  define TRACY_ALLOC_S( ptr, size, depth ) TRACY_ALLOC( ptr, size )
#  define TRACY_FREE_S( ptr, depth ) TRACY_FREE( ptr )
#  define TRACY_SECURE_ALLOC_S( ptr, size, depth ) TRACY_SECURE_ALLOC( ptr, size )
#  define TRACY_SECURE_FREE_S( ptr, depth ) TRACY_SECURE_FREE( ptr )

#  define TRACY_ALLOC_NS( ptr, size, depth, name ) TRACY_ALLOC( ptr, size, name )
#  define TRACY_FREE_NS( ptr, depth, name ) TRACY_FREE( ptr, name )
#  define TRACY_SECURE_ALLOC_NS( ptr, size, depth, name ) TRACY_SECURE_ALLOC( ptr, size, name )
#  define TRACY_SECURE_FREE_NS( ptr, depth, name ) TRACY_SECURE_FREE( ptr, name )

#  define TRACY_MESSAGE_S( txt, size, depth ) TRACY_MESSAGE( txt, size )
#  define TRACY_MESSAGE_LS( txt, depth ) TRACY_MESSAGE_L( txt )
#  define TRACY_MESSAGE_CS( txt, size, color, depth ) TRACY_MESSAGE_C( txt, size, color )
#  define TRACY_MESSAGE_LCS( txt, color, depth ) TRACY_MESSAGE_LC( txt, color )
#endif

#define TRACY_PARAMETER_REGISTER( cb ) tracy::Profiler::ParameterRegister( cb );
#define TRACY_PARAMETER_SETUP( idx, name, isBool, val ) tracy::Profiler::ParameterSetup( idx, name, isBool, val );
#define TRACY_IS_CONNECTED tracy::GetProfiler().IsConnected()

#ifdef TRACY_FIBERS
#  define TRACY_FIBER_ENTER( fiber ) tracy::Profiler::EnterFiber( fiber );
#  define TRACY_FIBER_LEAVE tracy::Profiler::LeaveFiber();
#endif

#endif
