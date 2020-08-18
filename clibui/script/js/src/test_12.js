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
                this.left = sys.get_config('screen/width') - this.width;
                this.top = sys.get_config('screen/height') - this.height;
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
            weight: 1
        });
    }
    bar.push(create_button('显示日志'));
    bar.push(create_button('复制控制台'));
    bar.push(create_button('暂停'));
    bar.push(create_button('重启'));
    bar.push(create_button('清理缓存'));
    bar.event.emit('resize', bar);
    UI.root.push(bar);
})();