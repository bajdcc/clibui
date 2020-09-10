FS = {
    readFileSync: function(fn) {
        return sys.fs({
            method: 'readFileSync',
            filename: fn
        });
    }
};
sys.builtin(FS);