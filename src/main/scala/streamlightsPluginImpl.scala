
import java.awt.Color

import org.codeoverflow.chatoverflow.api.io.event.chat.twitch.TwitchChatMessageReceiveEvent
import org.codeoverflow.chatoverflow.api.plugin.{PluginImpl, PluginManager}

import scala.collection.mutable.ListBuffer

class streamlightsPluginImpl(manager: PluginManager) extends PluginImpl(manager) {

  private val twitchChat = require.input.twitchChat("twitchChat", "Twitch chat input", false)
  private val channel = require.parameter.string("channel", "Twitch Channel that should control your light", false)
  private val arduino = require.output.serial("arduino", "An Arduino connected through serial port", false)
  private val brightness = require.parameter.string("brightness", "Brightness of the Arduino", true)

  loopInterval = 10
  private val pulses = ListBuffer[Pulse]()
  private var i = 0

  override def setup(): Unit = {
    try {
      log("Startup!")
      twitchChat.get.setChannel(channel.get.get)
      val bright = if (brightness isSet) brightness.get.get.toFloat else 4.0f
      setBrightness(bright)
      log(s"Brightness: $bright")
      setColor(Color.decode("#4B367C"))
    } catch {
      case _: NumberFormatException => log("[ERROR] Could not parse brightness value!")
    }
    twitchChat.get.registerChatMessageReceiveEventHandler(onMessage)
  }

  private def setColor(color: Color): Unit = out.println("#STRIP:" + toRGBString(color))

  private def setBrightness(brightness: Float): Unit = out.println("#BRIGH:" + brightness)

  private def onMessage(e: TwitchChatMessageReceiveEvent): Unit = {
    log(s"[M] ${e.getMessage}")
    log(s"Color: ${e.getMessage.getAuthor.getColor}")
    pulses += Pulse(Color.decode(e.getMessage.getAuthor.getColor))
  }

  override def loop(): Unit = {
    if (i < 5) {
      i += 1
    } else {
      i = 0
      pulses.foreach(pulse => {
        if (pulse >> 1) pulses -= pulse
      })
    }
  }

  override def shutdown(): Unit = {}

  private def setColor(pixel: Int, color: Color): Unit = out.print(s"#PIXEL:$pixel,${toRGBString(color)}\n")

  private def out = arduino.get.getPrintStream

  private def toRGBString(color: Color): String = s"${color.getRed},${color.getGreen},${color.getBlue}"

  case class Pulse(color: Color) {
    private var index = 0

    def >>(pixels: Int): Boolean = {
      if (index >= 25) {
        true
      } else {
        setColor(index, color)
        index += pixels
        false
      }
    }
  }
}
