#include "SoyHttpConnection.h"




TSocketConnection::TSocketConnection(const std::string& ServerAddress,const std::string& ThreadName) :
	SoyWorkerThread	( ThreadName, SoyWorkerWaitMode::Sleep ),
	mSocket			( new SoySocket )
{
	Soy::SplitHostnameAndPort( mServerHostname, mServerPort, ServerAddress );
}


void TSocketConnection::Shutdown()
{
	if ( mSocket )
	{
		mSocket->Close();
	}
	
	if ( mReadThread )
	{
		mReadThread->WaitToFinish();
		mReadThread.reset();
	}
	
}


bool TSocketConnection::Iteration()
{
	auto& Socket = *mSocket;
	try
	{
		if ( !Socket.IsConnected() )
		{
			mConnectionRef = Socket.Connect( mServerHostname.c_str(), mServerPort );
			
			//	if blocking and returns invalid, then the socket has probably error'd
			if ( !mConnectionRef.IsValid() )
			{
				throw Soy::AssertException("Connecton failed");
			}
			
			FlushRequestQueue();
			
			if ( !mReadThread )
			{
				mReadThread = CreateReadThread( mSocket, mConnectionRef );
			}
		}
	}
	catch ( std::exception& e )
	{
		Shutdown();
		std::stringstream Error;
		Error << "Failed to create & connect TCP socket: " << e.what();
		auto ErrorStr = Error.str();
		mOnError( ErrorStr );
		return true;
	}
	
	//	only wake up when something happens now
	SetWakeMode( SoyWorkerWaitMode::Wake );
	return true;
}

void TSocketConnection::SendRequest(std::shared_ptr<Soy::TWriteProtocol> Request)
{
	mRequestQueue.PushBack( Request );
	FlushRequestQueue();
}

void TSocketConnection::FlushRequestQueue()
{
	//	gotta wait for a connection!
	if ( !mSocket || !mConnectionRef.IsValid() )
		return;
	
	//	make thread if we don't have one
	if ( !mWriteThread )
	{
		mWriteThread = CreateWriteThread( mSocket, mConnectionRef );
		if ( !mWriteThread )
			return;
	}
	
	while ( !mRequestQueue.IsEmpty() )
	{
		auto QueueRequest = mRequestQueue.PopAt(0);
		std::shared_ptr<Soy::TWriteProtocol> Request = QueueRequest;
		mWriteThread->Push( Request );
	}
}



THttpConnection::THttpConnection(const std::string& Url) :
	TSocketConnection	( Soy::ExtractServerFromUrl(Url), "THttpConnection" )
{
	std::Debug << "Split Http fetch to server=" << mServerHostname << ":" << mServerPort << std::endl;
}

std::shared_ptr<TSocketReadThread> THttpConnection::CreateReadThread(std::shared_ptr<SoySocket> Socket,SoyRef ConnectionRef)
{
	auto OnResponse = [this](std::shared_ptr<Soy::TReadProtocol>& pProtocol)
	{
#if defined(ENABLE_RTTI)
		auto pHttpProtocol = std::dynamic_pointer_cast<Http::TResponseProtocol>( pProtocol );
#else
		auto pHttpProtocol = std::static_pointer_cast<Http::TResponseProtocol>( pProtocol );
#endif
		mOnResponse( pHttpProtocol );
	};
	
	std::shared_ptr<TSocketReadThread> ReadThread( new THttpReadThread( Socket, ConnectionRef ) );
	ReadThread->mOnDataRecieved = OnResponse;
	ReadThread->Start();
	return ReadThread;
}

std::shared_ptr<TSocketWriteThread> THttpConnection::CreateWriteThread(std::shared_ptr<SoySocket> Socket,SoyRef ConnectionRef)
{
	std::shared_ptr<TSocketWriteThread> WriteThread( new THttpWriteThread(Socket,ConnectionRef) );
	
	auto OnError = [this](const std::string& Error)
	{
	};
	
	WriteThread->mOnStreamError = OnError;
	WriteThread->Start();
	return WriteThread;
}

void THttpConnection::SendRequest(std::shared_ptr<Http::TRequestProtocol> Request)
{
	Soy::Assert( Request != nullptr, "Request expected" );
#if defined(ENABLE_RTTI)
	auto pRequest = std::dynamic_pointer_cast<Soy::TWriteProtocol>( Request );
#else
	auto pRequest = std::static_pointer_cast<Soy::TWriteProtocol>( Request );
#endif
	
	//	set host automatically
	if ( Request->mHost.empty() )
	{
		Request->mHost = mServerHostname;	//	was address, should this include port? Pretty sure, not.
	}
	
	TSocketConnection::SendRequest( Request );
}


void THttpConnection::SendRequest(const Http::TRequestProtocol& Request)
{
	std::shared_ptr<Http::TRequestProtocol> pRequest( new Http::TRequestProtocol(Request) );
	SendRequest( pRequest );
}

