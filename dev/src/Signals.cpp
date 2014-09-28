/*-------------------------------------------------------------------------
File    : $Archive: Signals.cpp $
Author  : $Author: Fkling $
Version : $Revision: 1 $
Orginal : 2012-09-20, 15:50
Descr   : Implementation of the Signal container and the channel object

Modified: $Date: $ by $Author: Fkling $
---------------------------------------------------------------------------
TODO: [ -:Not done, +:In progress, !:Completed]
<pre>
</pre>


\History
- 27.09.14, FKling, Implementation
---------------------------------------------------------------------------*/
#include "yapt/logger.h"
#include "yapt/ySystem.h"
#include "yapt/ySystem_internal.h"

#ifdef WIN32
#define snprintf _snprintf
#endif

using namespace yapt;

Signals::Signals() : BaseInstance(kInstanceType_Signals){
  AddAttribute("name","signals");
}
Signals::~Signals() {
    // for(int i=0;i<executeObjects.size();i++) {
    //   delete executeObjects[i];
    // }
}
int Signals::GetNumChannels() {
	return channels.size();
}

ISignalChannel *Signals::GetChannel(int idx) {
	return dynamic_cast<ISignalChannel *>(channels.at(idx));
}
ISignalChannel *Signals::GetChannel(const char *name) {
	
	for(int i=0;i<channels.size();i++) {
		//Logger::GetLogger("Signals")->Debug("GetChannel, %d - %s", i, channels.at(i)->GetName());
		if (!strcmp(channels.at(i)->GetName(), name)) {
			return dynamic_cast<ISignalChannel *>(channels.at(i));
		}
	}
	return NULL;
}

ISignalChannel *Signals::AddChannel(int id, const char *name, double accuracy) {
	SignalChannel *channel = new SignalChannel(id, name, accuracy);
	channels.push_back(channel);
	return dynamic_cast<ISignalChannel *>(channel);
}

void Signals::RemoveChannel(int idChannel) {

}

void Signals::RemoveChannel(const char *name) {

}

//////////////
SignalChannel::SignalChannel() : BaseInstance(kInstanceType_SignalChannel) {
	this->id = 0;
	this->name = std::string("");
	this->accuracy = 1.0f;
	this->idxCurrentSignal = 0;
}

SignalChannel::SignalChannel(int id, const char *name, float accuracy) : BaseInstance(kInstanceType_SignalChannel) {
	this->id = id;
	this->name = std::string(name);
	this->accuracy = accuracy;
	this->idxCurrentSignal = 0;
}

SignalChannel::~SignalChannel() {

}

int SignalChannel::GetId() {
	return this->id;
}

const char *SignalChannel::GetName() {
	return name.c_str();
}

void SignalChannel::SetName(const char *name) {
	this->name = std::string(name);
}

int SignalChannel::GetNumSignals() {
	return signals.size();
}

ISignal *SignalChannel::GetSignalAt(int idx) {
	return dynamic_cast<ISignal *>(signals.at(idx));
}

ISignal *SignalChannel::AddSignal(double t, int sigvalue) {
	Signal *sig = new Signal(t, sigvalue);
	signals.push_back(sig);
	return dynamic_cast<ISignal *>(sig);
}

void SignalChannel::RegisterSinkObject(IPluginObjectInstance *sink) {
	sinks.push_back(sink);
}

void SignalChannel::SendToSinks(ISignal *signal) {
	for(int i=0;i<sinks.size();i++) {
		IPluginObjectInstance *inst = sinks.at(i);
		IPluginObject *extObject = inst->GetExtObject();
		extObject->Signal(id, name.c_str(), signal->GetValue(), signal->GetTime());
	}
}

ISignal *SignalChannel::RaiseNext(double t) {
	ILogger *pLogger = Logger::GetLogger("SignalChannel");
	if (idxCurrentSignal >= signals.size()) return NULL;

	Signal *sig = signals[idxCurrentSignal];
	if (t >= sig->GetTime()) {
		idxCurrentSignal++;
		// check if accuracy passed - if so, we skip this signal
		if ((accuracy > 0.0f) && (sig->GetTime() > (t+accuracy))) {
			return NULL;
		}
		return dynamic_cast<ISignal *>(sig);
	}
	return NULL;
}

void SignalChannel::RemoveSignal(int idx) {

}

///////////////////
Signal::Signal(float t, int val) : BaseInstance(kInstanceType_Signal) {
	this->t = t;
	this->value = val;
}

Signal::Signal() : BaseInstance(kInstanceType_Signal) {
	this->t = -1.0;
	this->value = 0;
}

Signal::~Signal() {

}

double Signal::GetTime() {
	return t;
}

int Signal::GetValue() {
	return value;
}
