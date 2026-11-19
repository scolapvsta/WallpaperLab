// Wallpaper Lab background controls. Login form and authentication remain upstream.
import QtQuick
import QtMultimedia

Item {
    id: background
    property url source
    readonly property bool video: /\.(avi|mp4|mov|mkv|m4v|webm)$/i.test(source.toString())
    clip: true
    Item {
        id: media
        anchors.fill: parent
        AnimatedImage {
            anchors.fill: parent
            source: background.video ? "" : background.source
            fillMode: Image.Stretch
            asynchronous: true
            playing: true
        }
        MediaPlayer {
            id: player
            source: background.video ? background.source : ""
            videoOutput: videoOutput
            autoPlay: background.video
            loops: MediaPlayer.Infinite
            onPlayingChanged: if (playing) console.log("Video started.")
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
    }
    ShaderEffect {
        anchors.fill: parent
        property variant source: pixels
        property vector2d imageSize: Qt.vector2d(Number(config.LabWidth), Number(config.LabHeight))
        property vector2d outputSize: Qt.vector2d(width, height)
        property vector3d basic: Qt.vector3d(Number(config.LabBrightness), Number(config.LabContrast), Number(config.LabSaturation))
        property vector4d more: Qt.vector4d(Number(config.LabHue)*Math.PI/180, Number(config.LabTemperature), Number(config.LabGamma), Number(config.LabVignette))
        property vector3d geometry: Qt.vector3d(Number(config.LabRotation)*Math.PI/180, Number(config.LabFlipX)>0.5?-1:1, Number(config.LabFlipY)>0.5?-1:1)
        property vector3d placement: Qt.vector3d(Number(config.LabX), Number(config.LabY), Number(config.LabZoom))
        property real crop: config.CropBackground === "true" ? 1 : 0
        fragmentShader: "greeter.frag.qsb"
        onStatusChanged: if (status === ShaderEffect.Error) console.error("WALLPAPER_LAB_SHADER_ERROR", log)
    }
}
