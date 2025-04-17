window.addEventListener('load', () => {
    'use strict'

    let byId = id => document.getElementById(id)
    let ws = new WebSocket('ws://' + window.location.host + '/ws')
    ws.binaryType = 'arraybuffer'

    let hoursUpBtn = byId('hoursUp')
    let hoursDownBtn = byId('hoursDown')
    let minsUpBtn = byId('minsUp')
    let minsDownBtn = byId('minsDown')
    let secsUpBtn = byId('secsUp')
    let secsDownBtn = byId('secsDown')

    ws.addEventListener('message', e => {
        console.log('WS msg: ' + e)
    })

    // Add event handlers for buttons
    const upButtons = {
        0: hoursUpBtn,
        1: minsUpBtn,
        2: secsUpBtn
    }
    const downButtons = {
        0: hoursDownBtn,
        1: minsDownBtn,
        2: secsDownBtn
    }
    const identifiers = {
        0: 'h',
        1: 'm',
        2: 's'
    }

    function handleClick(is_up_button, is_mouse_down, ident) {
        console.log(identifiers[ident] + ' ' +
                    (is_up_button ? 'up' : 'down ') + ': ' + 
                    (is_mouse_down ? 'start' : 'stop'))
        let dview = new DataView(new ArrayBuffer(2))
        dview.setUint8(0, 0)
        let arg = 0
        if (is_up_button)
            arg |= 128
        if (is_mouse_down)
            arg |= 64
        arg |= ident
        dview.setUint8(1, arg)
        ws.send(dview.buffer)
    }

    for (const ident in upButtons) {
        if (upButtons.hasOwnProperty(ident)) {
            const upBtn = upButtons[ident]
            upBtn.addEventListener('pointerdown', () => {
                handleClick(true, true, ident)
            })
            upBtn.addEventListener('pointerup', () => {
                handleClick(true, false, ident)
            })
            const downBtn = downButtons[ident]
            downBtn.addEventListener('pointerdown', () => {
                handleClick(false, true, ident)
            })
            downBtn.addEventListener('pointerup', () => {
                handleClick(false, false, ident)
            })
        }
    }
})

