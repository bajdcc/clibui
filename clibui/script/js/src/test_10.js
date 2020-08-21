(function() {
    var moving = new UI({
        type: 'rect',
        color: 'rgb(1,162,255)',
        left: 10,
        top: 10,
        width: 50,
        height: 50,
        orientation: 'horizontal',
        event: new Event({
            'resize': UI.get_layout("linear")
        })
    });
    UI.root.push(moving);
    moving.push(new UI({
        type: 'rect',
        color: 'rgb(0,122,204)',
        margin: {
            left: 2,
            top: 2,
            right: 2,
            bottom: 2
        },
        weight: 1
    }));
    moving.push(new UI({
        type: 'rect',
        color: 'rgb(197,197,61)',
        margin: {
            left: 2,
            top: 2,
            right: 2,
            bottom: 2
        },
        weight: 2
    }));
    moving.event.emit('resize', moving);
    var moving_n = 5;
    setTimeout(function m() {
        moving.left += 10;
        moving.width += 10;
        moving.event.emit('resize', moving);
        if (moving_n-- > 0) {
            setTimeout(m, 200);
        }
    }, 200);
    sys.http({
        method: 'get',
        url: "http://www.sucaijishi.com/uploadfile/2015/0210/20150210104951657.gif",
        callback: function(data) {
            if (data.result === "success" && data.code === 200) {
                var image = new UI({
                    type: 'image',
                    left: 120,
                    top: 250,
                    width: 50,
                    height: 50,
                    full: true,
                    data: data.body
                });
                UI.root.push(image);
            }
        }
    })
})();