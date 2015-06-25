#ifndef INCLUDE_PLUGINAARDVARK_HPP_
#define INCLUDE_PLUGINAARDVARK_HPP_

#define PORT 0
#define HANDLE 1

#include <vector>
#include "ProcessInterface.hpp"
#include "JsonRPC.hpp"
#include "OutgoingMsg.hpp"
#include "RemoteAardvark.hpp"


/**
 * \class AardvarkCareTaker
 *
 */
class AardvarkCareTaker : public ProcessInterface{

	public:

		/** Base-constructor.*/
		AardvarkCareTaker();


		/**Base-destructor.*/
		virtual ~AardvarkCareTaker();


		/** Initializes mutex for the static device list.*/
		static void init();

		/** Destroys mutex for the static device list.*/
		static void deInit();


		//valueType can be PORT or HANDLE
		/**
		 * Opens or finds a instance of RemoteAardvark. A new instance of RemoteAardvark
		 * can only be opened with a port.
		 * \param value A port or handle number.
		 * \param valueType Specifies the type of value: 0 is PORT and 1 is HANDLE
		 * \return A instance of RemoteAardvark to the requested Aardvark
		 * \throws Error If the requested RemoteAardvark is already used by another user.
		 */
		RemoteAardvark* getDevice(int value, int valueType);

		/**
		 * Analyzes the incoming message and executes a requested function of RemoteAardvark.
		 * Only json rpc requests or notification can be processed by AardvarkCareTaker.
		 * Response or anything else will be discarded. Notifications are only used for binding
		 * a RemoteAardvark to a ConnectionContext. If a request was received, it will be analyzed if it
		 * has either a member "port" or a member "Aardvark". If none of this members is transmitted, it will
		 * be handled as a function which doesn't need a handle or port.
		 * \param input The incoming message we want to process.
		 * \return Outgoing message containing a json rpc response or error response.
		 */
		OutgoingMsg* process(IncomingMsg* input);

	private:
		/*! Json rpc parser for analyzing incoming messages.*/
		JsonRPC* json;
		/*! DOM for parsing/analyzing a received message.*/
		Document* currentDom;
		/*! Json RPC id value of the last received message.*/
		Value* id;
		/*! Unique identifier of the ConnectionContext of RSD which is connected to this AardvarkCareTaker.*/
		int contextNumber;
		/*! A Instance of RemoteAardvark which was initialized with -1 and can only execute functions which do not need a handle.*/
		RemoteAardvark* deviceLessFunctions;


		/*! Containing the instances of RemoteAardvark representing the real Aardvark adapters.*/
		static list<RemoteAardvark*> deviceList;


		/*! Mutex for protecting the deviceList. Because several instances of AardvarkCareTaker can access it.*/
		static pthread_mutex_t dLmutex;


		/*! Deletes the deviceList. All instances of RemoteAardvark will be deallocated and popped from the list.*/
		static void deleteDeviceList();

		/*! Unlocks all RemoteAardvark which where used by this AardvarkCareTaker.*/
		void unlockAllUsedDevices();
};


#endif /* INCLUDE_PLUGINAARDVARK_HPP_ */
