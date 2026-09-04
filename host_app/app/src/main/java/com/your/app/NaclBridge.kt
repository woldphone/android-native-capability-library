package com.your.app.nacl

import kotlinx.coroutines.flow.MutableSharedFlow
import kotlinx.coroutines.flow.asSharedFlow
import java.nio.ByteBuffer
import java.nio.ByteOrder

/**
 * High-performance, reactive JVM gateway consuming the Unified NACL API.
 */
object NaclBridge {
    // Standardized Module Mapping matching NaclModuleId enum
    const val MODULE_CORE = 0
    const val MODULE_BLUETOOTH = 1
    const val MODULE_WIFI = 2
    const val MODULE_SENSORS = 7

    // Thread-safe, hot broadcast stream for native telemetry events
    private val _events = MutableSharedFlow<NaclEvent>(extraBufferCapacity = 1000)
    val events = _events.asSharedFlow()

    init {
        // Load the compiled JNI and dynamic core coordinator
        System.loadLibrary("native_host_bridge")
    }

    // Java-side representation of the native NaclEventFrame C-struct
    data class NaclEvent(
        val moduleId: Int,
        val eventType: Int,
        val timestampNs: Long,
        val payload: ByteArray
    )

    /**
     * Entry point called directly by the native C++ event loop thread.
     * Bypasses heavy allocation paths by reusing preallocated ByteBuffers.
     */
    @JvmStatic
    private fun onNativeEvent(moduleId: Int, eventType: Int, timestampNs: Long, rawPayload: ByteBuffer) {
        rawPayload.order(ByteOrder.nativeOrder())
        val payloadBytes = ByteArray(rawPayload.remaining())
        rawPayload.get(payloadBytes)

        val event = NaclEvent(moduleId, eventType, timestampNs, payloadBytes)

        // Emits the event asynchronously to all active coroutine collectors
        _events.tryEmit(event)
    }

    // Native JNI methods mapping to our C API
    external fun initialize(privateDirPath: String): Boolean
    external fun shutdown()
    external fun setMockMode(moduleId: Int, enabled: Boolean)
    external fun dispatchCommand(moduleId: Int, cmdId: Int, payload: ByteArray): Int
    external fun getCapabilityMatrix(): String
}
