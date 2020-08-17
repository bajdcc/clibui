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
    var qr = new UI({
        type: 'qr',
        color: 'rgb(0,122,204)',
        background: 'rgba(0,0,0,0)',
        left: 120,
        top: 100,
        width: 100,
        height: 100,
        text: "https://github.com/bajdcc"
    });
    UI.root.push(qr);
})();