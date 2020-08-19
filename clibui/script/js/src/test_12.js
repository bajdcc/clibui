(function() {
    var bar = new UI({
        type: "empty",
        left: 0,
        top: 0,
        width: 600,
        height: 40,
        orientation: 'horizontal',
        event: new Event({
            'resize': function() {
                this.left = UI.root.width - this.width;
                this.top = UI.root.height - this.height;
                UI.get_layout("linear").call(this);
            }
        })
    });

    function create_button(text, callback) {
        return UI.button({
            text: text,
            font_size: 20,
            margin: {
                left: 5,
                top: 5,
                right: 5,
                bottom: 5
            },
            weight: 1,
            click: callback
        });
    }
    bar.push(create_button('显示日志', function() {
        this.show = this.show || false;
        this.show = !this.show;
        this.event.emit('text', this, this.show ? '隐藏日志' : '显示日志');
        sys.config({
            'sys': {
                'show_log': this.show
            }
        });
    }));
    bar.push(create_button('复制控制台', function() {
        sys.config({
            'sys': {
                'copy_console': true
            }
        });
    }));
    bar.push(create_button('暂停', function() {
        this.stop = this.stop || false;
        this.stop = !this.stop;
        this.event.emit('text', this, this.stop ? '继续' : '暂停');
        sys.config({
            'sys': {
                'set_running_state': !this.stop
            }
        });
    }));
    bar.push(create_button('重启', function() {
        sys.config({
            'sys': {
                'reboot': true
            }
        });
    }));
    bar.push(create_button('清理缓存', function() {
        sys.config({
            'sys': {
                'clear_cache': true
            }
        });
    }));
    bar.event.emit('resize', bar);
    UI.root.push(bar);
})();