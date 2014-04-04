extern "C" {
#   include "../game/q_shared.h"
#   include "../qcommon/qcommon.h"
#   include "../win32/win_shared.h"
}

#include <ppl.h>
#include <ppltasks.h>
#include <assert.h>

#include "win8_msgq.h"
#include "win8_app.h"

using namespace Windows::Networking;
using namespace Windows::Networking::Connectivity;
using namespace Windows::Networking::Sockets;

static qboolean networkingEnabled = qfalse;

static cvar_t	*net_noudp;


/*
=============
Sys_StringToAdr

idnewt
192.246.40.70
=============
*/
WIN8_EXPORT qboolean Sys_StringToAdr( const char *s, netadr_t *a ) {
    // @pjb: todo
    return qfalse;
}

/*
==================
Sys_GetPacket

Never called by the game logic, just the system event queing
==================
*/
WIN8_EXPORT qboolean Sys_GetPacket( netadr_t *net_from, msg_t *net_message ) {
    // @pjb: todo
    return qfalse;
}

/*
==================
Sys_SendPacket
==================
*/
WIN8_EXPORT void Sys_SendPacket( int length, const void *data, netadr_t to ) {
    // @pjb: todo
}

/*
==================
Sys_IsLANAddress

LAN clients will have their rate var ignored
==================
*/
WIN8_EXPORT qboolean Sys_IsLANAddress( netadr_t adr ) {
    // @pjb: todo
    return qfalse;
}

/*
==================
Sys_ShowIP
==================
*/
WIN8_EXPORT void Sys_ShowIP(void) {
    // @pjb: todo
}

/*
=====================
NET_StartListeningOnPort
=====================
*/
qboolean NET_StartListeningOnPort( HostName^ localHost, int port )
{
    return qfalse;
}

/*
====================
NET_StartListening
====================
*/
void NET_StartListening( void ) {

	cvar_t* ip = Cvar_Get( "net_ip", "localhost", CVAR_LATCH );
	int port = Cvar_Get( "net_port", va( "%i", PORT_SERVER ), CVAR_LATCH )->integer;

    HostName^ localHost = nullptr;

    auto hostNames = NetworkInformation::GetHostNames();
    if ( !hostNames->Size )
    {
        Com_Printf( "No adapters.\n" );
        return;
    }

    // First, see if there's an adapter that uses this port
    char hostNameTmp[256];
    for ( int i = 0; *ip->string && localHost == nullptr && i < hostNames->Size; ++i )
    {
        auto localHostInfo = hostNames->GetAt(i);
        if ( localHostInfo->IPInformation != nullptr )
        {
            Win8_CopyString( localHostInfo->DisplayName, hostNameTmp, sizeof( hostNameTmp ) );
            if ( Q_strncmp( ip->string, hostNameTmp, sizeof(hostNameTmp) ) == 0 )
            {
                localHost = localHostInfo;
            }
        }
    }

    // If there was no explicitly specified IP, pick the first adapter
    for ( int i = 0; localHost == nullptr && i < hostNames->Size; ++i )
    {
        auto localHostInfo = hostNames->GetAt(i);
        if ( localHostInfo->IPInformation != nullptr )
        {
            localHost = localHostInfo;
        }
    }

    // No adapter with IP information
    if ( localHost == nullptr )
    {
        Com_Printf( "ERROR: Found %d adapters but none have IP info.\n", hostNames->Size );
        return;
    }

    Win8_CopyString( localHost->DisplayName, hostNameTmp, sizeof( hostNameTmp ) );
    Com_Printf( "Found localHost %s.\n", hostNameTmp );

    Cvar_Set( "net_ip", hostNameTmp );

	// automatically scan for a valid port, so multiple
	// dedicated servers can be started without requiring
	// a different net_port for each one
	for( int i = 0 ; i < 10 ; i++ ) {
		if ( NET_StartListeningOnPort( localHost, port + i ) ) {
			Cvar_SetValue( "net_port", port + i );
			return;
		}
	}
	Com_Printf( "WARNING: Couldn't allocate IP port\n");
}
/*
====================
NET_StopListening
====================
*/
void NET_StopListening( void ) {
    // @pjb: todo
}

/*
====================
NET_GetCvars
====================
*/
static qboolean NET_GetCvars( void ) {
	qboolean	modified;

	modified = qfalse;

	if( net_noudp && net_noudp->modified ) {
		modified = qtrue;
	}
	net_noudp = Cvar_Get( "net_noudp", "0", CVAR_LATCH | CVAR_ARCHIVE );

	return modified;
}

/*
====================
NET_Config
====================
*/
void NET_Config( qboolean enableNetworking ) {

	qboolean	modified;
	qboolean	stop;
	qboolean	start;

	// get any latched changes to cvars
	modified = NET_GetCvars();

	if( net_noudp->integer ) {
		enableNetworking = qfalse;
	}

	// if enable state is the same and no cvars were modified, we have nothing to do
	if( enableNetworking == networkingEnabled && !modified ) {
		return;
	}

	if( enableNetworking == networkingEnabled ) {
		if( enableNetworking ) {
			stop = qtrue;
			start = qtrue;
		}
		else {
			stop = qfalse;
			start = qfalse;
		}
	}
	else {
		if( enableNetworking ) {
			stop = qfalse;
			start = qtrue;
		}
		else {
			stop = qtrue;
			start = qfalse;
		}
		networkingEnabled = enableNetworking;
	}

	if( stop ) {
		NET_StopListening();
	}

	if( start ) {
		NET_StartListening();
	}
}


/*
====================
NET_Init
====================
*/
WIN8_EXPORT void NET_Init( void ) {

    // @pjb: todo

    // Print the local addresses and adapters
	Com_Printf( "Init WinRT Sockets\n" );

	Com_Printf( "... [INFO] Set net_adapterId to specify a use a specific adapter.\n" );
	Com_Printf( "... [INFO] Set net_ip to specify a use a specific local address.\n" );
	Com_Printf( "... [INFO] Set net_port to specify a port to listen on.\n" );

	cvar_t* adapterId = Cvar_Get( "net_adapterId", "", CVAR_LATCH );
	cvar_t* ip = Cvar_Get( "net_ip", "localhost", CVAR_LATCH );
    
    char hostnameTmp[256];
    char adapterIdTmp[256];
    // @pjb: todo: fatal error C1001: An internal error has occurred in the compiler.
    // for each ( HostName^ localHostInfo in NetworkInformation::GetHostNames() )
    auto hostNames = NetworkInformation::GetHostNames();
    for ( int i = 0; i < hostNames->Size; ++i )
    {
        auto localHostInfo = hostNames->GetAt(i);
        if ( localHostInfo->IPInformation != nullptr )
        {
            Win8_CopyString( localHostInfo->DisplayName, hostnameTmp, sizeof( hostnameTmp ) );
            Win8_CopyString( localHostInfo->IPInformation->NetworkAdapter->NetworkAdapterId.ToString(), adapterIdTmp, sizeof( adapterIdTmp ) );

            Com_Printf( "... Address: %s / Adapter: %s\n", hostnameTmp, adapterIdTmp );

            // If net_adapterId is set, compare and set if necessary
            if ( *adapterId->string && !*ip->string && Q_strncmp( adapterId->string, adapterIdTmp, sizeof(adapterIdTmp) ) == 0 )
            {
                Cvar_Set( "net_ip", hostnameTmp );
            }
        }
    }

    // Configure the addresses
	NET_Config( qtrue );
}

/*
====================
NET_Shutdown
====================
*/
WIN8_EXPORT void NET_Shutdown( void ) {

    // @pjb: todo

	NET_Config( qfalse );
}

/*
====================
NET_Sleep

sleeps msec or until net socket is ready
====================
*/
WIN8_EXPORT void NET_Sleep( int msec ) {
    // @pjb: nothing to do here.
}

/*
====================
NET_Restart_f
====================
*/
WIN8_EXPORT void NET_Restart( void ) {
	NET_Config( networkingEnabled );
}
