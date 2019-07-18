import org.codeoverflow.chatoverflow.api.plugin.{Pluggable, Plugin, PluginManager}

class streamlights extends Pluggable {

  override def getName: String = "streamlights"

  override def getAuthor: String = "joblo2213"

  override def getDescription: String = "Light control through twitch chat"

  override def getMajorAPIVersion: Int = 2

  override def getMinorAPIVersion: Int = 0

  override def createNewPluginInstance(manager: PluginManager): Plugin = new streamlightsPluginImpl(manager)
}
