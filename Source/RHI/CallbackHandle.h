#pragma once

class CallbackHandler {
public:
	using Callback = void(*)(void* user);

	/**
	 * Schedules the callback to be called onto the appropriate thread.
	 * Typically this will be the application's main thead.
	 *
	 * Must be thread-safe.
	 */
	virtual void post(void* user, Callback callback) = 0;

protected:
	virtual ~CallbackHandler() = default;
};