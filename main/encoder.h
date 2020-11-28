/**
 * Button state
 */
typedef enum {
    RE_BTN_RELEASED = 0,      //!< Button currently released
    RE_BTN_PRESSED = 1,       //!< Button currently pressed
    RE_BTN_LONG_PRESSED = 2   //!< Button currently long pressed
} rotary_encoder_btn_state_t;

/**
 * Rotary encoder descriptor
 */
typedef struct
{
    gpio_num_t pin_a, pin_b, pin_btn; //!< Encoder pins. pin_btn can be >= GPIO_NUM_MAX if no button used
    uint8_t code;
    uint16_t store;
    size_t index;
    uint64_t btn_pressed_time_us;
    rotary_encoder_btn_state_t btn_state;
} rotary_encoder_t;

/**
 * Event type
 */
typedef enum {
    RE_ET_CHANGED = 0,      //!< Encoder turned
    RE_ET_BTN_RELEASED,     //!< Button released
    RE_ET_BTN_PRESSED,      //!< Button pressed
    RE_ET_BTN_LONG_PRESSED, //!< Button long pressed (press time (us) > RE_BTN_LONG_PRESS_TIME_US)
    RE_ET_BTN_CLICKED       //!< Button was clicked
} rotary_encoder_event_type_t;
