package com.your.app.nacl

import kotlinx.coroutines.flow.MutableSharedFlow
import kotlinx.coroutines.flow.asSharedFlow
import java.nio.ByteBuffer

object NaclUsbBridge {
    private val _usbEvents = MutableSharedFlow<ByteArray>(extraBufferCapacity = 500)
    val usbEvents = _usbEvents.asSharedFlow()

    init {
        System.loadLibrary("libusb_client")
    }

    /**
     * Entry point invoked directly by the native C++ bulk transfer completion thread.
     */
    @JvmStatic
    private fun onBulkDataReceived(rawBuffer: ByteBuffer, size: Int) {
        val payload = ByteArray(size)
        rawBuffer.get(payload)
        _usbEvents.tryEmit(payload)
    }

    external fun initializeUsb(fd: Int, interfaceNum: Int, epIn: Int, epOut: Int): Int
    external fun sendBulkPayload(data: ByteArray): Int
    external fun shutdownUsb()
}
