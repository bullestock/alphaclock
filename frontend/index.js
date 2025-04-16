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
        /*
        let dview = new DataView(e.data)
        let command = dview.getUint8(0)
        if (command === 3 && e.data.byteLength === 3) {
            let batteryValue = dview.getInt16(1, true) / 1000
            battery.innerText = batteryValue.toFixed(2)
        } else if (command === 2 && e.data.byteLength === 5) {
            tiltX = dview.getInt16(3, true) / 32768.0
            tiltY = dview.getInt16(1, true) / 32768.0
            }
        */
    })

    // Add event handlers for buttons
    const upButtons = {
        'h': hoursUpBtn,
        'm': minsUpBtn,
        's': secsUpBtn
    }
    const downButtons = {
        'h': hoursDownBtn,
        'm': minsDownBtn,
        's': secsDownBtn
    }

    function handleClick(is_up_button, is_mouse_down, ident) {
        console.log(ident + ' ' +
                    (is_up_button ? 'up' : 'down ') + ': ' + 
                    (is_mouse_down ? 'start' : 'stop'))
    }

    for (const ident in upButtons) {
        if (upButtons.hasOwnProperty(ident)) {
            const upBtn = upButtons[ident]
            upBtn.addEventListener('mousedown', () => {
                handleClick(true, true, ident)
            })
            upBtn.addEventListener('mouseup', () => {
                handleClick(true, false, ident)
            })
            const downBtn = downButtons[ident]
            downBtn.addEventListener('mousedown', () => {
                handleClick(false, true, ident)
            })
            downBtn.addEventListener('mouseup', () => {
                handleClick(false, false, ident)
            })
        }
    }

    function sendSteeringCommand(turning, speed) {
        let dview = new DataView(new ArrayBuffer(3))
        dview.setUint8(0, 0)
        dview.setInt8(1, turning * 127)
        dview.setInt8(2, speed * 127)
        ws.send(dview.buffer)
    }

})

