// Wallpaper Lab background controls. Login form and authentication remain upstream.
import QtQuick
import QtMultimedia
import "LabLayout.js" as LabLayout

Item {
    id: background
    property url source
    property rect screenRect: Qt.rect(Screen.virtualX, Screen.virtualY, Screen.width, Screen.height)
    property rect desktopRect: LabLayout.desktop(Qt.application.screens)
    readonly property bool panorama: config.LabDisplayMode === "span"
    readonly property bool scene: config.LabScene === "true"
    property double frameTick: 0
    property bool sceneReceived: false
    readonly property bool video: /\.(avi|mp4|mov|mkv|m4v|webm)$/i.test(source.toString())
    clip: true
    Timer {
        interval: 33; running: background.scene; repeat: true
        onTriggered: background.frameTick = Math.floor(Date.now()/33)
    }
    function syncVideo() {
        if (!panorama || !player.seekable || player.duration <= 0) return;
        var target = Math.max(0, Date.now()-Number(config.LabVideoEpoch)) % player.duration;
        if (Math.abs(target-player.position)>100) player.position=target;
    }
    Timer { interval: 250; running: background.video && background.panorama; repeat: true; onTriggered: background.syncVideo() }
    Item {
        id: media
        anchors.fill: parent
        AnimatedImage {
            anchors.fill: parent
            source: background.video || background.scene ? "" : background.source
            fillMode: Image.Stretch
            asynchronous: true
            playing: true
        }
        Image {
            anchors.fill: parent
            source: background.scene ? Qt.resolvedUrl(config.LabSceneFrame)+"?frame="+background.frameTick : ""
            fillMode: Image.Stretch
            cache: false
            asynchronous: true
            retainWhileLoading: true
            onStatusChanged: if (status === Image.Ready && !background.sceneReceived) { background.sceneReceived=true; console.log("WALLPAPER_LAB_SCENE_READY"); }
        }
        MediaPlayer {
            id: player
            source: background.video ? background.source : ""
            videoOutput: videoOutput
            autoPlay: background.video
            loops: MediaPlayer.Infinite
            onPlayingChanged: if (playing) { console.log("Video started."); Qt.callLater(background.syncVideo); }
        }
        VideoOutput {
            id: videoOutput
            anchors.fill: parent
            visible: background.video
            fillMode: VideoOutput.Stretch
        }
    }
    ShaderEffectSource {
        id: pixels
        sourceItem: media
        hideSource: true
        live: true
        property real textureScale: Math.min(1,3840/Math.max(1,Number(config.LabWidth)),2160/Math.max(1,Number(config.LabHeight)))
        textureSize: Qt.size(Math.max(1,Number(config.LabWidth)*textureScale), Math.max(1,Number(config.LabHeight)*textureScale))
    }
    ShaderEffect {
        anchors.fill: parent
        property variant source: pixels
        property vector2d imageSize: Qt.vector2d(Number(config.LabWidth), Number(config.LabHeight))
        property vector2d outputSize: background.panorama ? Qt.vector2d(background.desktopRect.width, background.desktopRect.height) : Qt.vector2d(width, height)
        property vector4d region: background.panorama ? Qt.vector4d((background.screenRect.x-background.desktopRect.x)/background.desktopRect.width, (background.screenRect.y-background.desktopRect.y)/background.desktopRect.height, background.screenRect.width/background.desktopRect.width, background.screenRect.height/background.desktopRect.height) : Qt.vector4d(0,0,1,1)
        property vector3d basic: Qt.vector3d(Number(config.LabBrightness), Number(config.LabContrast), Number(config.LabSaturation))
        property vector4d more: Qt.vector4d(Number(config.LabHue)*Math.PI/180, Number(config.LabTemperature), Number(config.LabGamma), Number(config.LabVignette))
        property vector3d geometry: Qt.vector3d(Number(config.LabRotation)*Math.PI/180, Number(config.LabFlipX)>0.5?-1:1, Number(config.LabFlipY)>0.5?-1:1)
        property vector3d placement: Qt.vector3d(Number(config.LabX), Number(config.LabY), Number(config.LabZoom))
        property real crop: config.CropBackground === "true" ? 1 : 0
        fragmentShader: "greeter.frag.qsb"
        onStatusChanged: if (status === ShaderEffect.Error) console.error("WALLPAPER_LAB_SHADER_ERROR", log)
    }
}
