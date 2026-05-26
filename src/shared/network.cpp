// partner.steamgames.com/doc/api/ISteamnetworkingSockets
// github.com/ValveSoftware/GameNetworkingSockets/blob/master/examples/example_chat.cpp

#include "network.h"
#include "types.h"
#include <stdio.h>

#include <GameNetworkingSockets/steam/steamnetworkingtypes.h>
#include <GameNetworkingSockets/steam/steamnetworkingsockets_flat.h>
#include <sys/types.h>

// SERVER ==============================================================================
HSteamNetPollGroup m_hPollGroup;
ISteamNetworkingSockets* m_pInterface;

/**
static ChatServer *s_pCallbackInstance;
static void SteamNetConnectionStatusChangedCallback( SteamNetConnectionStatusChangedCallback_t *pInfo ){
        s_pCallbackInstance->OnSteamNetConnectionStatusChanged( pInfo );
}
**/
void OnConnectionStatusChanged(SteamNetConnectionStatusChangedCallback_t *pInfo){
        char temp[1024];
        switch(pInfo->m_info.m_eState){
                case k_ESteamNetworkingConnectionState_None:
                        // NOTE: We will get callbacks here when we destroy connections.  You can ignore these.
                        break;

                case k_ESteamNetworkingConnectionState_ClosedByPeer:
                case k_ESteamNetworkingConnectionState_ProblemDetectedLocally:
                {
                        // Ignore if they were not previously connected.  (If they disconnected
                        // before we accepted the connection.)
                        if ( pInfo->m_eOldState == k_ESteamNetworkingConnectionState_Connected ){

                                // Locate the client.  Note that it should have been found, because this
                                // is the only codepath where we remove clients (except on shutdown),
                                // and connection change callbacks are dispatched in queue order.
                                auto itClient = m_mapClients.find( pInfo->m_hConn );
                                assert( itClient != m_mapClients.end() );

                                // Select appropriate log messages
                                const char *pszDebugLogAction;
                                if ( pInfo->m_info.m_eState == k_ESteamNetworkingConnectionState_ProblemDetectedLocally ){
                                        pszDebugLogAction = "problem detected locally";
                                        sprintf( temp, "Alas, %s hath fallen into shadow.  (%s)", itClient->second.m_sNick.c_str(), pInfo->m_info.m_szEndDebug );
                                }
                                else{
                                        // Note that here we could check the reason code to see if
                                        // it was a "usual" connection or an "unusual" one.
                                        pszDebugLogAction = "closed by peer";
                                        sprintf( temp, "%s hath departed", itClient->second.m_sNick.c_str() );
                                }

                                // Spew something to our own log.  Note that because we put their nick
                                // as the connection description, it will show up, along with their
                                // transport-specific data (e.g. their IP address)
                                printf( "Connection %s %s, reason %d: %s\n",
                                        pInfo->m_info.m_szConnectionDescription,
                                        pszDebugLogAction,
                                        pInfo->m_info.m_eEndReason,
                                        pInfo->m_info.m_szEndDebug
                                );

                                m_mapClients.erase( itClient );

                                // Send a message so everybody else knows what happened
                                SendStringToAllClients( temp );
			}else{
                                assert( pInfo->m_eOldState == k_ESteamNetworkingConnectionState_Connecting );
                        }

                        // Clean up the connection.  This is important!
                        // The connection is "closed" in the network sense, but
                        // it has not been destroyed.  We must close it on our end, too
                        // to finish up.  The reason information do not matter in this case,
                        // and we cannot linger because it's already closed on the other end,
                        // so we just pass 0's.
                        m_pInterface->CloseConnection( pInfo->m_hConn, 0, nullptr, false );
                        break;
                }

                case k_ESteamNetworkingConnectionState_Connecting:
                {
                        // This must be a new connection
                        assert( m_mapClients.find( pInfo->m_hConn ) == m_mapClients.end() );

                        printf( "Connection request from %s", pInfo->m_info.m_szConnectionDescription );

                        // A client is attempting to connect
                        // Try to accept the connection.
                        if ( m_pInterface->AcceptConnection( pInfo->m_hConn ) != k_EResultOK ){
                                // This could fail.  If the remote host tried to connect, but then
                                // disconnected, the connection may already be half closed.  Just
                                // destroy whatever we have on our side.
                                m_pInterface->CloseConnection( pInfo->m_hConn, 0, nullptr, false );
                                printf( "Can't accept connection.  (It was already closed?)" );
                                break;
                        }

                        // Assign the poll group
                        if ( !m_pInterface->SetConnectionPollGroup( pInfo->m_hConn, m_hPollGroup ) ){
                                m_pInterface->CloseConnection( pInfo->m_hConn, 0, nullptr, false );
                                printf( "Failed to set poll group?" );
                                break;
                        }

                        // Generate a random nick.  A random temporary nick
                        // is really dumb and not how you would write a real chat server.
                        // You would want them to have some sort of signon message,
                        // and you would keep their client in a state of limbo (connected,
                        // but not logged on) until them.  I'm trying to keep this example
                        // code really simple.
                        char nick[ 64 ];
                        sprintf( nick, "BraveWarrior%d", 10000 + ( rand() % 100000 ) );

                        // Send them a welcome message
                        sprintf( temp, "Welcome, stranger.  Thou art known to us for now as '%s'; upon thine command '/nick' we shall know thee otherwise.", nick ); 
                        SendStringToClient( pInfo->m_hConn, temp ); 

                        // Also send them a list of everybody who is already connected
                        if ( m_mapClients.empty() ){
                                SendStringToClient( pInfo->m_hConn, "Thou art utterly alone." ); 
                        }
                        else{
                                sprintf( temp, "%d companions greet you:", (int)m_mapClients.size() ); 
                                for ( auto &c: m_mapClients )
                                        SendStringToClient( pInfo->m_hConn, c.second.m_sNick.c_str() ); 
				}

				// Let everybody else know who they are for now
				sprintf( temp, "Hark!  A stranger hath joined this merry host.  For now we shall call them '%s'", nick ); 
				SendStringToAllClients( temp, pInfo->m_hConn ); 

				// Add them to the client list, using std::map wacky syntax
				m_mapClients[ pInfo->m_hConn ];
				SetClientNick( pInfo->m_hConn, nick );
				break;
                }

                case k_ESteamNetworkingConnectionState_Connected:
                        // We will get a callback immediately after accepting the connection.
                        // Since we are the server, we can ignore this, it's not news to us.
                        break;

                default:
                        // Silences -Wswitch
                        break;
        }
}

NetSocket net_listen(u_int16_t port){
        HSteamListenSocket m_hListenSock;
        SteamNetworkingIPAddr ip_addr;
        SteamNetworkingConfigValue_t options[3];

        m_pInterface = SteamNetworkingSockets();

        ip_addr.Clear();
        ip_addr.m_port = port;
        
        // Allocate space for 3 custom options
        // 1. Hook up the status callback
        options[0].SetPtr(k_ESteamNetworkingConfig_Callback_ConnectionStatusChanged, (void*)OnConnectionStatusChanged);
        // 2. Allow raw IP connections without Steam account authentication
        options[1].SetInt32(k_ESteamNetworkingConfig_IP_AllowWithoutAuth, 1);
        // 3. Simulate a realistic bad connection (5% packet loss) for engine testing
        options[2].SetFloat(k_ESteamNetworkingConfig_FakePacketLoss_Send, 0.05f);

        m_hListenSock = m_pInterface->CreateListenSocketIP( ip_addr, 3, options);

        if( m_hListenSock == k_HSteamListenSocket_Invalid ){
                fprintf(stderr, "𐄂 Failed to listen on port %d", port);
        }
        m_hPollGroup = m_pInterface->CreatePollGroup();
        if ( m_hPollGroup == k_HSteamNetPollGroup_Invalid ){
                fprintf(stderr, "𐄂 Failed to listen on port %d", port);
        }
        printf( "✓ Server listening on port %d\n", port );

        return (NetSocket) m_hListenSock;
}

// CLIENT ==============================================================================

