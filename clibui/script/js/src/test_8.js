(function() {
    UI.root.push(
        new UI({
            type: 'label',
            color: '#ffffff',
            content: 'Hello world!',
            left: 10,
            top: 60,
            width: 100,
            height: 20,
            align: 'center',
            valign: 'center',
            font: {
                family: 'KaiTi',
                size: 16
            }
        })
    );
    var block = new UI({
        type: 'rect',
        color: '#ff0000',
        left: 10,
        top: 100,
        width: 100,
        height: 100,
        hit: true,
        event: new Event({
            'hit': function(t, x, y) {
                console.log(this.type, this.left, this.top, this.color, t, x, y);
            }
        })
    });
    UI.root.push(
        block
    );
    UI.root.push(
        new UI({
            type: 'round',
            color: '#ff0000',
            left: 10,
            top: 210,
            width: 100,
            height: 100,
            radius: 5,
            hit: true,
            event: new Event({
                'hit': function(t, x, y) {
                    console.log(this.type, this.left, this.top, this.color, t, x, y);
                }
            })
        })
    );
    var i = 0;
    var colors = [
        [255, 0, 0],
        [255, 0, 20],
        [255, 0, 40],
        [255, 0, 60],
        [255, 0, 80],
        [255, 0, 100],
        [255, 0, 120],
        [255, 0, 140],
        [255, 0, 160],
        [255, 0, 180],
        [255, 0, 200],
        [255, 0, 220],
        [255, 0, 240],
        [240, 0, 255],
        [220, 0, 255],
        [200, 0, 255],
        [180, 0, 255],
        [160, 0, 255],
        [140, 0, 255],
        [120, 0, 255],
        [100, 0, 255],
        [80, 0, 255],
        [60, 0, 255],
        [40, 0, 255],
        [20, 0, 255],
        [0, 0, 255],
        [0, 0, 255],
        [0, 20, 255],
        [0, 40, 255],
        [0, 60, 255],
        [0, 80, 255],
        [0, 100, 255],
        [0, 120, 255],
        [0, 140, 255],
        [0, 160, 255],
        [0, 180, 255],
        [0, 200, 255],
        [0, 220, 255],
        [0, 240, 255],
        [0, 255, 240],
        [0, 255, 220],
        [0, 255, 200],
        [0, 255, 180],
        [0, 255, 160],
        [0, 255, 140],
        [0, 255, 120],
        [0, 255, 100],
        [0, 255, 80],
        [0, 255, 60],
        [0, 255, 40],
        [0, 255, 20],
        [0, 255, 0],
        [0, 255, 0],
        [20, 255, 0],
        [40, 255, 0],
        [60, 255, 0],
        [80, 255, 0],
        [100, 255, 0],
        [120, 255, 0],
        [140, 255, 0],
        [160, 255, 0],
        [180, 255, 0],
        [200, 255, 0],
        [220, 255, 0],
        [240, 255, 0]
    ];
    var tid = setInterval(function() {
        if (i >= colors.length) {
            return clearInterval(tid);
        }
        block.color = 'rgb(' + colors[i][0] + ',' + colors[i][1] + ',' + colors[i][2] + ')';
        i++;
    }, 100);
})();