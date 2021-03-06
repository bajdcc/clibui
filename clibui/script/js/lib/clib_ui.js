UI.prototype.render = function(n, l) {
    if (n === 1)
        this.render_internal(n, l);
    this.forEach(x => {
        if (x && x instanceof UI) {
            x.render(n, l + 1);
        }
    });
    if (n === 2)
        this.render_internal(n, l);
};
UI.prototype.resize = function() {
    if (this.event && this.event.has('resize')) {
        this.event.emit('resize', this);
    } else {
        this.map(x => x.resize());
    }
};
UI.root = new UI({
    type: 'root',
    left: 0,
    top: 0,
    width: 0,
    height: 0
});
UI.get_layout = (function() {
    const map_layout = {
        linear: function() {
            if (!this.orientation)
                this.orientation = 'vertical';
            var w = this.map(x => x.weight || 0);
            var all = w.reduce((a, b) => a + b, 0);
            if (all === 0)
                return;
            all = 1 / all;
            var sum = 0;
            if (this.orientation == 'horizontal') {
                for (var i in this) {
                    var o = this[i];
                    o.left = this.left + sum;
                    o.width = Math.ceil(this.width * w[i] * all);
                    sum += o.width;
                    o.top = this.top;
                    o.height = this.height;
                    if (o.margin) {
                        if (o.margin.left && o.margin.left > 0) {
                            o.left += o.margin.left;
                            o.width -= o.margin.left;
                        }
                        if (o.margin.top && o.margin.top > 0) {
                            o.top += o.margin.top;
                            o.height -= o.margin.top;
                        }
                        if (o.margin.right && o.margin.right > 0) o.width -= o.margin.right;
                        if (o.margin.bottom && o.margin.bottom > 0) o.height -= o.margin.bottom;
                    }
                    if (o.event)
                        o.event.emit('resize', o);
                }
            } else if (this.orientation == 'vertical') {
                for (var i in this) {
                    var o = this[i];
                    o.top = this.top + sum;
                    o.height = Math.ceil(this.height * w[i] * all);
                    sum += o.height;
                    o.left = this.left;
                    o.width = this.width;
                    if (o.margin) {
                        if (o.margin.left) {
                            o.left += o.margin.left;
                            o.width -= o.margin.left;
                        }
                        if (o.margin.top) {
                            o.top += o.margin.top;
                            o.height -= o.margin.top;
                        }
                        if (o.margin.right) o.width -= o.margin.right;
                        if (o.margin.bottom) o.height -= o.margin.bottom;
                    }
                    if (o.event)
                        o.event.emit('resize', o);
                }
            }
        },
        fill: function() {
            for (var i in this) {
                var o = this[i];
                o.left = this.left;
                o.width = this.width;
                o.top = this.top;
                o.height = this.height;
                if (o.margin) {
                    if (o.margin.left && o.margin.left > 0) {
                        o.left += o.margin.left;
                        o.width -= o.margin.left;
                    }
                    if (o.margin.top && o.margin.top > 0) {
                        o.top += o.margin.top;
                        o.height -= o.margin.top;
                    }
                    if (o.margin.right && o.margin.right > 0) o.width -= o.margin.right;
                    if (o.margin.bottom && o.margin.bottom > 0) o.height -= o.margin.bottom;
                }
                if (o.event)
                    o.event.emit('resize', o);
            }
        }
    };
    return function(t) {
        return map_layout[t];
    }
})();
sys.send_signal = (function() {
    const map_signal = {
        render: function() {
            if (UI.root && UI.root instanceof UI) {
                UI.root.render(1, 0);
                UI.root.render(2, 0);
            }
        },
        resize: function() {
            if (UI.root && UI.root instanceof UI) {
                UI.root.width = sys.get_config('screen/width');
                UI.root.height = sys.get_config('screen/height');
                UI.root.resize();
            }
        },
        hit: function() {
            var obj = sys.get_config('hit/obj');
            var type = sys.get_config('hit/type');
            var x = sys.get_config('hit/x');
            var y = sys.get_config('hit/y');
            var wheel = sys.get_config('hit/wheel');
            if (obj && obj.event) {
                obj.event.emit('hit', obj, type, x, y, wheel);
            }
        }
    };
    return function(n) {
        return map_signal[n]();
    }
})();
sys.send_signal("resize");
sys.exec_file("ui/clib_ui_button.js");
return;