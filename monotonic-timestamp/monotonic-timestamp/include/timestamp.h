//
// Created by 陈鹏飞 on 2020/2/19.
//

#ifndef SCUTTLEBUTT_TIMESTAMP_H
#define SCUTTLEBUTT_TIMESTAMP_H

// If `system_clock::now()` is invoked twice quickly, it's possible to get two
// identical time stamps. To avoid generation duplications, subsequent
// calls are manually ordered to force uniqueness.

long timestamp();

#endif //SCUTTLEBUTT_TIMESTAMP_H
