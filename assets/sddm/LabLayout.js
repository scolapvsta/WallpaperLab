.pragma library

// SDDM exposes one screenModel per view; use Qt's complete screen list instead.
function desktop(screens) {
    if (!screens || !screens.length) return Qt.rect(0, 0, 1280, 720);
    var left=Infinity, top=Infinity, right=-Infinity, bottom=-Infinity;
    for (var i=0; i<screens.length; ++i) {
        var s=screens[i];
        left=Math.min(left,s.virtualX); top=Math.min(top,s.virtualY);
        right=Math.max(right,s.virtualX+s.width); bottom=Math.max(bottom,s.virtualY+s.height);
    }
    return Qt.rect(left,top,Math.max(1,right-left),Math.max(1,bottom-top));
}
