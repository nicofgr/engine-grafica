// partner.steamgames.com/doc/api/ISteamnetworkingSockets
// github.com/ValveSoftware/GameNetworkingSockets/blob/master/examples/example_chat.cpp

#include "network.h"
#include "types.h"
#include <stdio.h>

#include <GameNetworkingSockets/steam/steamnetworkingsockets.h>
#include <GameNetworkingSockets/steam/steamnetworkingsockets_flat.h>
#include <GameNetworkingSockets/steam/steamnetworkingtypes.h>
#include <sys/types.h>

enum Role {
        UNDEFINED,
        SERVER,
        CLIENT
};

enum Role role = UNDEFINED;
SteamNetworkingMicroseconds g_logTimeZero;
HSteamNetPollGroup m_hPollGroup;
ISteamNetworkingSockets* m_pInterface;
bool g_bQuit = false;

// SERVER ==============================================================================

typedef struct ClientArray{
        HSteamNetConnection* array;
        u32 size;
}ClientArray;

ClientArray client_array;

static inline void client_array_push(HSteamNetConnection conn){
        if(client_array.size == 0){
                client_array.array = (HSteamNetConnection*)malloc(sizeof(HSteamNetConnection));
        }else{
                client_array.array = (HSteamNetConnection*)realloc(client_array.array, sizeof(HSteamNetConnection)*client_array.size+1);
        }
        client_array.array[client_array.size] = conn;
        client_array.size++;
}

void SendStringToClient( HSteamNetConnection conn, const char *str ){
        m_pInterface->SendMessageToConnection( conn, str, (uint32)strlen(str), k_nSteamNetworkingSend_Reliable, nullptr );
}

void ServerOnConnectionStatusChanged(SteamNetConnectionStatusChangedCallback_t *pInfo){
        char temp[1024];
        switch(pInfo->m_info.m_eState){
                case k_ESteamNetworkingConnectionState_None:
                        // NOTE: We will get callbacks here when we destroy connections.  You can ignore these.
                        break;

                case k_ESteamNetworkingConnectionState_ClosedByPeer:
                case k_ESteamNetworkingConnectionState_ProblemDetectedLocally:
                {
                        // Ignore if they were not previously connected.  
                        // (If they disconnected before we accepted the connection.)
                        if ( pInfo->m_eOldState == k_ESteamNetworkingConnectionState_Connected ){

                                // Locate the client.  Note that it should have been found, because this
                                // is the only codepath where we remove clients (except on shutdown),
                                // and connection change callbacks are dispatched in queue order.
                                //auto itClient = m_mapClients.find( pInfo->m_hConn );
                               // assert( itClient != m_mapClients.end() );

                                // Select appropriate log messages
                                const char *pszDebugLogAction;
                                if ( pInfo->m_info.m_eState == k_ESteamNetworkingConnectionState_ProblemDetectedLocally ){
                                        pszDebugLogAction = "problem detected locally\n";
                                        //sprintf( temp, "Alas, %s hath fallen into shadow.  (%s)", itClient->second.m_sNick.c_str(), pInfo->m_info.m_szEndDebug );
                                }
                                else{
                                        // Note that here we could check the reason code to see if
                                        // it was a "usual" connection or an "unusual" one.
                                        pszDebugLogAction = "closed by peer\n";
                                        //sprintf( temp, "%s hath departed", itClient->second.m_sNick.c_str() );
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

                                //m_mapClients.erase( itClient );

                                // Send a message so everybody else knows what happened
                                //SendStringToAllClients( temp );
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
                        //assert( m_mapClients.find( pInfo->m_hConn ) == m_mapClients.end() );

                        printf( "Connection request from %s\n", pInfo->m_info.m_szConnectionDescription );

                        // A client is attempting to connect
                        // Try to accept the connection.
                        if ( m_pInterface->AcceptConnection( pInfo->m_hConn ) != k_EResultOK ){
                                // This could fail.  If the remote host tried to connect, but then
                                // disconnected, the connection may already be half closed.  Just
                                // destroy whatever we have on our side.
                                m_pInterface->CloseConnection( pInfo->m_hConn, 0, nullptr, false );
                                printf( "Can't accept connection.  (It was already closed?)\n" );
                                break;
                        }

                        printf( "Connection accepted from %s\n", pInfo->m_info.m_szConnectionDescription );

                        // Assign the poll group
                        if ( !m_pInterface->SetConnectionPollGroup( pInfo->m_hConn, m_hPollGroup ) ){
                                m_pInterface->CloseConnection( pInfo->m_hConn, 0, nullptr, false );
                                printf( "Failed to set poll group?\n" );
                                break;
                        }

                        // Generate a random nick.
                        // You would want them to have some sort of signon message,
                        // and you would keep their client in a state of limbo (connected,
                        // but not logged on) until them.
                        char nick[ 64 ];
                        sprintf( nick, "BraveWarrior%d", 10000 + ( rand() % 100000 ) );

                        // Send them a welcome message
                        sprintf( temp, "Welcome, stranger.  Thou art known to us for now as '%s'; upon thine command '/nick' we shall know thee otherwise.", nick ); 
                        SendStringToClient( pInfo->m_hConn, temp ); 

                        // Also send them a list of everybody who is already connected
                        /**
                        if ( m_mapClients.empty() ){
                                //SendStringToClient( pInfo->m_hConn, "Thou art utterly alone." ); 
                        }else{
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
                        **/
                        client_array_push(pInfo->m_hConn);
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
        HSteamListenSocket    m_hListenSock;
        SteamNetworkingIPAddr ip_addr;
        SteamNetworkingConfigValue_t options[3];

        role = SERVER;

        m_pInterface = SteamNetworkingSockets();

        ip_addr.Clear();
        ip_addr.m_port = port;
        
        // Allocate space for 3 custom options
        // 1. Hook up the status callback
        options[0].SetPtr(k_ESteamNetworkingConfig_Callback_ConnectionStatusChanged, (void*)ServerOnConnectionStatusChanged);
        // 2. Allow raw IP connections without Steam account authentication
        options[1].SetInt32(k_ESteamNetworkingConfig_IP_AllowWithoutAuth, 1);
        // 3. Simulate a realistic bad connection (5% packet loss) for engine testing
        //options[2].SetFloat(k_ESteamNetworkingConfig_FakePacketLoss_Send, 0.05f);

        m_hListenSock = m_pInterface->CreateListenSocketIP( ip_addr, 2, options);

        if( m_hListenSock == k_HSteamListenSocket_Invalid ){
                fprintf(stderr, "𐄂 Failed to listen on port %d\n", port);
                exit(0);
        }
        m_hPollGroup = m_pInterface->CreatePollGroup();
        if ( m_hPollGroup == k_HSteamNetPollGroup_Invalid ){
                fprintf(stderr, "𐄂 Failed to listen on port %d\n", port);
                exit(0);
        }
        printf( "✓ Server listening on port %d\n", port );

        return (NetSocket) m_hListenSock;
}

// CLIENT ==============================================================================


HSteamNetConnection m_hConnection;

void ClientPollIncomingMessages(){
        while ( !g_bQuit ){
                ISteamNetworkingMessage *pIncomingMsg = nullptr;
                int numMsgs = m_pInterface->ReceiveMessagesOnConnection( m_hConnection, &pIncomingMsg, 1 );
                if ( numMsgs == 0 ){
                        break;
                }
                if ( numMsgs < 0 ){
                        fprintf(stderr, "Error checking for messages\n" );
                        exit(0);
                }

                // Just echo anything we get from the server
                fwrite( pIncomingMsg->m_pData, 1, pIncomingMsg->m_cbSize, stdout );
                fputc( '\n', stdout );

                // We don't need this anymore.
                pIncomingMsg->Release();
        }
}

void ClientOnConnectionStatusChanged( SteamNetConnectionStatusChangedCallback_t *pInfo ) {
        assert( pInfo->m_hConn == m_hConnection || m_hConnection == k_HSteamNetConnection_Invalid );

        // What's the state of the connection?
        switch ( pInfo->m_info.m_eState ){
                case k_ESteamNetworkingConnectionState_None:
                        // NOTE: We will get callbacks here when we destroy connections.  You can ignore these.
                        break;

                case k_ESteamNetworkingConnectionState_ClosedByPeer:
                case k_ESteamNetworkingConnectionState_ProblemDetectedLocally:
                {
                        g_bQuit = true;

                        // Print an appropriate message
                        if ( pInfo->m_eOldState == k_ESteamNetworkingConnectionState_Connecting ){
                                // Note: we could distinguish between a timeout, a rejected connection,
                                // or some other transport problem.
                                printf( "We sought the remote host, yet our efforts were met with defeat.  (%s)\n", pInfo->m_info.m_szEndDebug );
                        }else if ( pInfo->m_info.m_eState == k_ESteamNetworkingConnectionState_ProblemDetectedLocally ){
                                printf( "Alas, troubles beset us; we have lost contact with the host.  (%s)\n", pInfo->m_info.m_szEndDebug );
                        }else{
                                // NOTE: We could check the reason code for a normal disconnection
                                printf( "The host hath bidden us farewell.  (%s)\n", pInfo->m_info.m_szEndDebug );
                        }

                        // Clean up the connection.  This is important!
                        // The connection is "closed" in the network sense, but
                        // it has not been destroyed.  We must close it on our end, too
                        // to finish up.  The reason information do not matter in this case,
                        // and we cannot linger because it's already closed on the other end,
                        // so we just pass 0's.
                        m_pInterface->CloseConnection( pInfo->m_hConn, 0, nullptr, false );
                        m_hConnection = k_HSteamNetConnection_Invalid;
                        break;
                }

                case k_ESteamNetworkingConnectionState_Connecting:
                        // We will get this callback when we start connecting.
                        // We can ignore this.
                        break;

                case k_ESteamNetworkingConnectionState_Connected:
                        printf( "Connected to server OK\n" );
                        break;

                default:
                        // Silences -Wswitch
                        break;
        }
}

NetConn net_connect(u_int16_t port){
        SteamNetworkingIPAddr server_addr;

        role = CLIENT;

        // Select instance to use.  For now we'll always use the default.
        m_pInterface = SteamNetworkingSockets();

        server_addr.Clear();
        server_addr.ParseString("127.0.0.1");
        server_addr.m_port = port;

        // Start connecting
        char szAddr[ SteamNetworkingIPAddr::k_cchMaxString ];
        server_addr.ToString( szAddr, sizeof(szAddr), true );
        printf( "Connecting to server at %s\n", szAddr );
        SteamNetworkingConfigValue_t opt;
        opt.SetPtr( k_ESteamNetworkingConfig_Callback_ConnectionStatusChanged, (void*)ClientOnConnectionStatusChanged );
        m_hConnection = m_pInterface->ConnectByIPAddress( server_addr, 1, &opt );
        if ( m_hConnection == k_HSteamNetConnection_Invalid ){
                fprintf(stderr, "Failed to create connection\n" );
                exit(0);
        }
        return m_hConnection;
}

// BOTH ==============================================================================

static void DebugOutput( ESteamNetworkingSocketsDebugOutputType eType, const char *pszMsg ){
	SteamNetworkingMicroseconds time = SteamNetworkingUtils()->GetLocalTimestamp() - g_logTimeZero;
	printf( "%10.6f %s\n", time*1e-6, pszMsg );
	fflush(stdout);
	if ( eType == k_ESteamNetworkingSocketsDebugOutputType_Bug ){
		fflush(stdout);
		fflush(stderr);
		exit(1);
	}
}

void net_init(){
        SteamDatagramErrMsg errMsg;
        if ( !GameNetworkingSockets_Init( nullptr, errMsg ) ){
	        fprintf(stderr, "GameNetworkingSockets_Init failed.  %s\n", errMsg );
                exit(0);
        }
        g_logTimeZero = SteamNetworkingUtils()->GetLocalTimestamp();
        SteamNetworkingUtils()->SetDebugOutputFunction( k_ESteamNetworkingSocketsDebugOutputType_Msg, DebugOutput );

        client_array.array = NULL;
        client_array.size  = 0;
}

void net_update(){
        if(role == CLIENT){
                ClientPollIncomingMessages();
        }

        m_pInterface->RunCallbacks();
}

void net_send(Type type, u16 size, const void* buf){
        if(role == CLIENT)
                return;
        if(client_array.size == 0)
                return;

        HSteamNetConnection conn;
        conn = client_array.array[0];
        PacketHeader header;
        header.size = size;
        header.type = type;

        u32 total_size = sizeof(PacketHeader) + size;
        u8  stack_buffer[total_size];

        memcpy(stack_buffer, &header, sizeof(PacketHeader));
        memcpy(stack_buffer + sizeof(PacketHeader), buf, size);
        
        printf("Package size: %d bytes\n", header.size);
        m_pInterface->SendMessageToConnection(conn, stack_buffer, total_size, k_nSteamNetworkingSend_Reliable, nullptr);
}

void net_recv(){
}

void net_shutdown(){
        GameNetworkingSockets_Kill();
}
