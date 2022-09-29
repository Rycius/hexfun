enum game_event_type
{
    E_TILE_REVIELD,
};

typedef void (*event_func)(void *taget, void *data);

struct _game_event
{
    size_t hash;
    event_func e;
};

struct _event_store
{
    game_event_type key;
    _game_event *value;
};

_event_store *_events = 0;


void AddEventListener(game_event_type et, event_func e)
{
    _game_event *events = hmget(_events, et);
    
    size_t hash = stbds_hash_bytes(&e, sizeof(void *), 0);

    if(arrlen(events) > 0)
    {
        for(int32 i = 0; i < arrlen(events); ++i)
        {
            if(events[i].hash == hash)
            {
                TraceLog(LOG_INFO, "WARNING: Trying to add same event twice.");
                return;
            }
        }
    }
    
    arrpush(events, ((_game_event){hash, e}));

    hmput(_events, et, events);
}

void RemoveEventListener(game_event_type et, event_func e)
{
    _game_event *events = hmget(_events, et);

    if(arrlen(events) == 0)
    {
        TraceLog(LOG_INFO, "WARNING: Traing to remove event listener from empty list");
        return;
    }

    size_t hash = stbds_hash_bytes(&e, sizeof(void *), 0);

    for(int32 i = 0; i < arrlen(events); ++i)
    {
        if(events[i].hash == hash)
        {
            arrdel(events, i);

            if(arrlen(events) == 0)
            {
                hmdel(_events, et);
            }

            return;
        }
    }

    TraceLog(LOG_INFO, "WARNING: Traing to remove non existing event listener");
}

void TriggerEvent(game_event_type et, void *target, void *data)
{
    _game_event *events = hmget(_events, et);

    if(arrlen(events) == 0) return;

    for(int32 i = 0; i < arrlen(events); ++i)
    {
        events[i].e(target, data);
    }
}
