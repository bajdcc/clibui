UI.button = function(config) {
    var parent = new UI({
        type: 'empty',
        left: config.left,
        top: config.top,
        width: config.width,
        height: config.height,
        enable: true,
        enter: false,
        focused: false,
        hit: true,
        cursor: "hand",
        event: new Event({
            'resize': UI.get_layout("fill"),
            'text': function(t) {
                this[1].content = t;
            },
            'enable': function(t) {

            },
            'hit': function(t) {
                switch (t) {
                    case 'leftbuttondown':
                        this.focused = true;
                        this[0].color = this.bgcolor_focus;
                        this.event.emit('click', this);
                        break;
                    case 'leftbuttonup':
                        this.focused = false;
                        this[0].color = this.bgcolor;
                        break;
                    case 'mouseenter':
                        this.enter = true;
                        this[1].font.underline = true;
                        this[1].font = this[1].font;
                        this[1].color = this.fgcolor_focus;
                        break;
                    case 'mouseleave':
                        this.enter = false;
                        this[1].font.underline = false;
                        this[1].font = this[1].font;
                        this[1].color = this.fgcolor;
                        break;
                }
            }
        })
    });
    parent.bgcolor = config.bgcolor || '#CCCCCC';
    parent.bgcolor_focus = config.bgcolor_focus || '#FFFFFF';
    parent.fgcolor = config.fgcolor || '#333333';
    parent.fgcolor_focus = config.fgcolor_focus || '#000000';
    parent.fgcolor_disable = config.fgcolor_disable || '#888888';
    parent.text = config.text || 'Button';
    parent.font_size = config.font_size || 24;
    parent.family = config.font_family || '楷体';
    for (var i in config) {
        if (config.hasOwnProperty(i) && !parent.hasOwnProperty(i))
            parent[i] = config[i];
    }
    if (parent.click)
        parent.event.on('click', parent.click);
    var bg = new UI({
        type: 'round',
        color: parent.bgcolor,
        radius: 2
    });
    var fg = new UI({
        type: 'label',
        color: parent.fgcolor,
        content: parent.text,
        margin: {
            left: 2,
            top: 2,
            right: 2,
            bottom: 2
        },
        align: 'center',
        valign: 'center',
        font: {
            family: parent.family,
            size: parent.font_size
        }
    });
    parent.push(bg);
    parent.push(fg);
    parent.event.emit('resize', parent);
    return parent;
};