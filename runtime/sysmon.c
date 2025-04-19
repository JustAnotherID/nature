//
// Created by weiwenhao on 2023/11/16.
//

#include "sysmon.h"

#define CO_TIMEOUT (10 * 1000 * 1000) // ms

// 等待 1000ms 的时间，如果无法抢占则 deadlock
#define PREEMPT_TIMEOUT 1000

static void solo_processor_sysmon() {
    // - 监控状态处于 running solo processor, 需要做的工作有
    // 1. 遍历 solo_processor 需要先获取 solo processor lock
    // 2. 如果 solo processor exit 需要进行清理
    // 3. 如果 solo processor.need_stw == true, 则需要辅助 processor 进入 safe_point
    //        TRACEF("[wait_sysmon.solo] wait solo_processor_locker");
    //        mutex_lock(&solo_processor_locker);
    //        TRACEF("[wait_sysmon.solo] get solo_processor_locker, solo_p_count=%d", solo_processor_count);
    //
    //        n_processor_t *prev = NULL;
    //        n_processor_t *p = solo_processor_list;
    //        while (p) {
    //            if (p->status == P_STATUS_EXIT) {
    //                RDEBUGF("[wait_sysmon.solo] p=%p, index=%d, exited, prev=%p,  will remove from solo_processor_list=%p",
    //                        p, p->index, prev,
    //                        solo_processor_list);
    //
    //                n_processor_t *exited = p;
    //                if (prev) {
    //                    prev->next = p->next;
    //                    p = p->next;
    //                } else {
    //                    solo_processor_list = p->next;
    //                    p = p->next;
    //                }
    //
    //                processor_free(exited);
    //                RDEBUGF("[wait_sysmon.solo] p=%p remove from solo processor list, prev=%p, solo_processor_list=%p",
    //                        exited, prev,
    //                        solo_processor_list);
    //
    //                continue;
    //            }
    //
    //            // solo processor 仅需要 stw 的时候进行抢占, 由于锁定了 solo_processor_locker, 所以其他线程此时
    //            // 无法对 p->need_stw 进行解锁。
    //            if (p->need_stw == 0) {
    //                TRACEF("[wait_sysmon.solo] processor_index=%d, current not need stw, will skip", p->index);
    //
    //                prev = p;
    //                p = p->next;
    //                continue;
    //            }
    //
    //            // 不知道是不是自己的 stw 锁，难道需要重复解锁？什么时候解锁比较合适？processor_stop_stw() 之后？
    //            // 什么时候将 safe_point 配置为 false 呢？也在 processor_stop_stw 之后？
    //            if (processor_safe(p)) {
    //                RDEBUGF("[wait_sysmon.solo] processor_index=%d, in safe_point, will skip", p->index);
    //
    //                prev = p;
    //                p = p->next;
    //                continue;
    //            }
    //
    //            if (p->status != P_STATUS_RUNNING && p->status != P_STATUS_TPLCALL && p->status != P_STATUS_RTCALL) {
    //                RDEBUGF("[wait_sysmon.solo] p->status=%d, not running/tplcall/rtcall, p_index=%d, will skip", p->status,
    //                        p->index);
    //
    //                prev = p;
    //                p = p->next;
    //                continue;
    //            }
    //
    //            // 仅仅超时才考虑进行抢占或者辅助 safe_point
    //            uint64_t co_start_at = p->co_started_at;
    //            if (co_start_at == 0) {
    //                RDEBUGF("[wait_sysmon.share] p_index=%d, co=%p co_stared_at = 0, will skip", p->index, p->coroutine);
    //                continue;
    //            }
    //            uint64_t time = (uv_hrtime() - co_start_at);
    //            if (time < CO_TIMEOUT) {
    //                RDEBUGF("[wait_sysmon.share] p_index=%d, co=%p run not timeout(%lu ms), will skip", p->index,
    //                        p->coroutine,
    //                        time / 1000 / 1000);
    //                continue;
    //            }
    //
    //            RDEBUGF("[wait_sysmon.solo] run timeout, will get thread_locker, p_index_%d=%d(%lu)", p->share, p->index,
    //                    (uint64_t) p->thread_id);
    //            continue; // TODO 暂时取消辅助 GC，以协作式 GC 为主
    //
    //
    //            int trylock = mutex_times_trylock(&p->thread_locker, 10);
    //            if (trylock == -1) {
    //                RDEBUGF("[wait_sysmon.solo] trylock failed, p_index_%d=%d(%lu), p_status=%d", p->share, p->index,
    //                        (uint64_t) p->thread_id,
    //                        p->status);
    //                continue;
    //            }
    //
    //            DEBUGF("[wait_sysmon.solo.thread_locker] success get thread_locker, p_index_%d=%d(%lu), p_status=%d",
    //                   p->share, p->index,
    //                   (uint64_t) p->thread_id, p->status);
    //
    //            if (p->need_stw == 0) {
    //                RDEBUGF("[wait_sysmon.solo.thread_locker] not need stw, p_index_%d=%d(%lu), will goto unlock", p->share,
    //                        p->index,
    //                        (uint64_t) p->thread_id);
    //
    //                goto SOLO_UNLOCK_NEXT;
    //            }
    //
    //            if (processor_safe(p)) {
    //                RDEBUGF("[wait_sysmon.solo.thread_locker] already in safe_point, p_index_%d=%d(%lu), will goto unlock",
    //                        p->share, p->index,
    //                        (uint64_t) p->thread_id);
    //
    //                goto SOLO_UNLOCK_NEXT;
    //            }
    //
    //            if (p->status != P_STATUS_RUNNING && p->status != P_STATUS_TPLCALL && p->status != P_STATUS_RTCALL) {
    //                DEBUGF(
    //                        "[wait_sysmon.solo.thread_locker] p->status=%d, not running/tplcall/rtcall, p_index=%d, goto unlock",
    //                        p->status,
    //                        p->index);
    //
    //                goto SOLO_UNLOCK_NEXT;
    //            }
    //
    //            co_start_at = p->co_started_at;
    //            if (co_start_at == 0) {
    //                DEBUGF("[wait_sysmon.solo.thread_locker] p_index=%d, p_status=%d, co_stared_at = 0, goto unlock",
    //                       p->index, p->status);
    //                goto SOLO_UNLOCK_NEXT;
    //            }
    //            time = (uv_hrtime() - co_start_at);
    //            if (time < CO_TIMEOUT) {
    //                DEBUGF("[wait_sysmon.solo.thread_locker] p_index=%d, not run timeout(%lu ms),  goto unlock", p->index,
    //                       time / 1000 / 1000);
    //                goto SOLO_UNLOCK_NEXT;
    //            }
    //
    //            DEBUGF(
    //                    "[wait_sysmon.solo.thread_locker] p_index=%d run timeout(%lu ms) need assist to safe_point, will exclude rtcall status",
    //                    p->index, time / 1000 / 1000);
    //
    //            // 禁止 rtcall 状态
    //            while (true) {
    //                // rtcall -> running/dispatch
    //                if (p->status != P_STATUS_RTCALL) {
    //                    break;
    //                }
    //
    //                // 进入到 STW 安全状态
    //                if (p->status == P_STATUS_DISPATCH) {
    //                    DEBUGF(
    //                            "[wait_sysmon.solo.thread_locker] p_index=%d(%lu), p_status=%d co=%p change dispatch, will unlock",
    //                            p->index,
    //                            (uint64_t) p->thread_id, p->status, p->coroutine);
    //                    goto SOLO_UNLOCK_NEXT;
    //                }
    //
    //                usleep(1 * 1000);
    //            }
    //
    //            DEBUGF(
    //                    "[wait_sysmon.solo.thread_locker] p_index=%d(%lu), p_status=%d co=%p not eq rtcall, will get gc_stw_locker",
    //                    p->index,
    //                    (uint64_t) p->thread_id, p->status, p->coroutine);
    //
    //            // gc_stw_locker 禁止 tplcall -> running
    ////            mutex_lock(&p->gc_solo_stw_locker);
    //            DEBUGF("[wait_sysmon.solo.thread_locker] success get gc_stw_locker, p_index_%d=%d(%lu), p_status=%d",
    //                   p->share, p->index, (uint64_t) p->thread_id, p->status);
    //
    //            assert(p->status == P_STATUS_RUNNING || p->status == P_STATUS_TPLCALL || p->status == P_STATUS_DISPATCH);
    //
    //            DEBUGF("[wait_sysmon.solo.thread_locker] p_index=%d(%lu), p_status=%d co=%p not eq rtcall", p->index,
    //                   (uint64_t) p->thread_id,
    //                   p->status, p->coroutine);
    //
    //            // 判断完以后可能已经进入到了 dispatch, 进入到 dispatch 意味着可以主动进入 stw 状态, 但是此时预缓存了 p->need_stw,
    //            // 倒也不用担心开启了新一轮的 gc
    //            uint64_t need_stw = p->need_stw;
    //            if (p->status == P_STATUS_TPLCALL) {
    //                // gc_solo_stw_locker 获取成功， 辅助进入 gc 状态
    //                if (p->safe_point < need_stw) {
    //                    p->safe_point = need_stw;
    //                }
    //                DEBUGF(
    //                        "[wait_sysmon.solo.thread_locker] p_index=%d(%lu), p_status=%d co=%p in tplcall, assist to safe point, goto unlock",
    //                        p->index, (uint64_t) p->thread_id, p->status, p->coroutine);
    //                goto SOLO_UNLOCK_NEXT;
    //            }
    //
    //            if (p->status == P_STATUS_DISPATCH) {
    //                DEBUGF("[wait_sysmon.solo.thread_locker] p_index=%d(%lu), p_status=%d co=%p in dispatch, goto unlock",
    //                       p->index,
    //                       (uint64_t) p->thread_id, p->status, p->coroutine);
    //                goto SOLO_UNLOCK_NEXT;
    //            }
    //
    //            assert(p->status == P_STATUS_RUNNING);
    //
    //            DEBUGF(
    //                    "[wait_sysmon.solo.thread_locker] p_index=%d(%lu),p_status=%d co=%p running timeout will send SIGURG",
    //                    p->index,
    //                    (uint64_t) p->thread_id, p->status, p->coroutine);
    //
    //            if (pthread_kill(p->thread_id, SIGURG) != 0) {
    //                assert(false && "error sending SIGURG to thread");
    //            }
    //
    //            DEBUGF("[wait_sysmon.solo.thread_locker] p_index=%d send SIGURG success", p->index);
    //            p->status = P_STATUS_PREEMPT;
    //        SOLO_UNLOCK_NEXT:
    //            mutex_unlock(&p->thread_locker);
    //
    //            DEBUGF("[wait_sysmon.solo.thread_locker] unlocker, p_index_%d=%d", p->share, p->index);
    //
    //            prev = p;
    //            p = p->next;
    //        }
    //
    //        mutex_unlock(&solo_processor_locker);
    //        DEBUGF("[wait_sysmon.solo] solo_processor_locker unlock");
}

static void processor_sysmon() {
    // - 监控长时间被占用的 share processor 进行抢占式调度
    PROCESSOR_FOR(processor_list) {
        // 没有需要运行的 runnable_list(等待运行的 runnable) 并且当前也不需要 stw 则不需要则不考虑抢占
        if (p->need_stw == 0 && p->runnable_list.count == 0) {
            DEBUGF("[processor_sysmon] p_index=%d p_status=%d runnable_list.count == 0 cannot preempt, will skip", p->index, p->status);
            continue;
        }

        // 还未随 thread 初始化完成
        if (!p->tls_yield_safepoint_ptr) {
            DEBUGF("[processor_sysmon] p_index=%d p_status=%d tls_yield_safepoint_ptr is null cannot preempt, will skip", p->index, p->status);
            continue;
        }

        // 已经设置过辅助 GC 不需要处理
        if (*p->tls_yield_safepoint_ptr) {
            DEBUGF("[processor_sysmon] p_index=%d p_status=%d tls_yield_safepoint_ptr is false cannot preempt, will skip", p->index, p->status);
            continue;
        }

        // running 既 coroutine running
        if (p->status != P_STATUS_RUNNING) {
            DEBUGF("[processor_sysmon] p_index=%d p_status=%d cannot preempt, will skip", p->index, p->status);
            continue;
        }

        coroutine_t *co = p->coroutine;
        assert(co); // p co 切换期间不会清空 co 了
        if (co->flag & FLAG(CO_FLAG_RTFN)) { // rt fn 包括 gc_work/signal_handle
            DEBUGF("[processor_sysmon] p_index=%d(%lu), co=%p is runtime fn, will skip", p->index,
                    (uint64_t) p->thread_id, p->coroutine);
            continue;
        }

        uint64_t co_start_at = p->co_started_at;
        if (co_start_at == 0) {
            DEBUGF("[wait_sysmon.share] p_index=%d, co=%p/%p co_stared_at = 0, will skip", p->index, p->coroutine,
                    co);
            continue;
        }
        uint64_t time = (uv_hrtime() - co_start_at);
        if (time < CO_TIMEOUT) {
            DEBUGF("[processor_sysmon] p_index=%d, co=%p/%p run not timeout(%lu ms), will skip", p->index,
                    p->coroutine, co,
                    time / 1000 / 1000);
            continue;
        }

        // 设置辅助 yield (如果 processor 进入 safepoint, 则会清空 safepoint)
        *p->tls_yield_safepoint_ptr = true;

        DEBUGF("[processor_sysmon] p_index=%d(%lu), co=%p run timeout=%d ms(co_start=%ld) set tls yield safepoint=true", p->index,
                (uint64_t) p->thread_id, p->coroutine, time / 1000 / 1000, co_start_at / 1000 / 1000);
    }


    //            RDEBUGF("[wait_sysmon.share] runtime timeout(%lu ms), wait locker, p_index_%d=%d(%lu)", time / 1000 / 1000,
    //                    p->share, p->index,
    //                    (uint64_t) p->thread_id);
    //
    //            RDEBUGF("warning deadlock: processor %d run timeout(%lu ms)", p->index, time / 1000 / 1000);
    //
    //
    //            // 尝试 10ms 获取抢占 disable_preempt_locker,避免抢占期间抢占状态变更成不可抢占, 如果获取不到就跳过
    //            // 但是可以从 not -> can
    //            int trylock = mutex_times_trylock(&p->thread_locker, 10);
    //            if (trylock == -1) {
    //                RDEBUGF("[wait_sysmon.share] trylock failed, p_index_%d=%d(%lu)", p->share, p->index,
    //                        (uint64_t) p->thread_id);
    //                continue;
    //            }
    //
    //            // yield/exit 首先就设置 can_preempt = false, 这里就无法完成。所以 p->coroutine 一旦设置就无法被清空或者切换
    //            RDEBUGF("[wait_sysmon.share.thread_locker] try locker success, p_index_%d=%d", p->share, p->index);
    //
    //            // 判断当前是否是可抢占状态
    //            if (p->status != P_STATUS_RUNNING && p->status != P_STATUS_TPLCALL && p->status != P_STATUS_RTCALL) {
    //                RDEBUGF("[wait_sysmon.share.thread_locker] p_index=%d, status=%d cannot preempt, goto unlock", p->index,
    //                        p->status);
    //                goto SHARE_UNLOCK_NEXT;
    //            }
    //
    //            // 获取了 thread_locker 此时协程的状态被锁定，且 p->co 也不再允许进行切换
    //            // 但是获取该锁期间，status/p->co 可能都发生了变化，所以需要重新进行判断
    //            // 成功获取到 locker 并且当前 can_preempt = true 允许抢占，
    //            // 但是这期间可能已经发生了 co 切换，所以再次进行 co_start 超时的判断
    //            co = p->coroutine;
    //            assert(co);
    //            // 协程发生了切换，原来的超时已经失效
    //            if (co->flag & FLAG(CO_FLAG_RTFN)) {
    //                RDEBUGF("[wait_sysmon.share.thread_locker] p_index=%d, co=%p is runtime fn, goto unlock", p->index, co);
    //                goto SHARE_UNLOCK_NEXT;
    //            }
    //
    //            // 由于获取了锁，此时 p->co_started_at 无法被重新赋值，所以再次获取时间判断超时
    //            co_start_at = p->co_started_at;
    //            if (co_start_at == 0) {
    //                RDEBUGF("[wait_sysmon.share.thread_locker] p_index=%d, co=%p co_stared_at = 0, goto unlock", p->index,
    //                        co);
    //                goto SHARE_UNLOCK_NEXT;
    //            }
    //            time = (uv_hrtime() - co_start_at);
    //            if (time < CO_TIMEOUT) {
    //                RDEBUGF("[wait_sysmon.share.thread_locker] p_index=%d, co=%p not timeout(%lu ms), goto unlock",
    //                        p->index, p->coroutine,
    //                        time / 1000 / 1000);
    //                goto SHARE_UNLOCK_NEXT;
    //            }
    //
    //            // 无限等待状态变成 running, 而不会主动抢占，但是会日志提醒主线程占用？还是强制抢占呢?
    //            // 只要 wait_sysmon 是干净的，强制抢占的话，时间是不是可以短一点, 另外 sprintf 总是可能卡死的
    //
    //            // 基于协作的原则，还是不进行强制抢占，还是使用 deadlock 进行提醒，时间可以设置久一点，500ms
    //            int count = 0;
    //            while (true) {
    //                // 进入到允许抢占状态
    //                if (p->status == P_STATUS_RUNNING) {
    //                    DEBUGF(
    //                            "[wait_sysmon.share.thread_locker] p_index=%d(%lu), p_status=%d co=%p in running, use count=%d",
    //                            p->index,
    //                            (uint64_t) p->thread_id, p->status, p->coroutine, count);
    //                    if (count > 200) {
    //                        DEBUGF(
    //                                "[wait_sysmon.share.thread_locker] p_index=%d(%lu), p_status=%d co=%p syscall timout use count=%d",
    //                                p->index,
    //                                (uint64_t) p->thread_id, p->status, p->coroutine, count);
    //                    }
    //                    break;
    //                }
    //
    //                // 进入到 STW 安全状态
    //                if (p->status == P_STATUS_DISPATCH) {
    //                    RDEBUGF(
    //                            "[wait_sysmon.share.thread_locker] p_index=%d(%lu), p_status=%d co=%p change dispatch, will unlock",
    //                            p->index,
    //                            (uint64_t) p->thread_id, p->status, p->coroutine);
    //                    goto SHARE_UNLOCK_NEXT;
    //                }
    //
    //                count++;
    //                usleep(1 * 1000);
    //            }
    //
    //            // 抢占超时
    //            // if (p->status == P_STATUS_TPLCALL || p->status == P_STATUS_RTCALL) {
    //            //     assertf(false, "deadlock: syscall run timeout, p_index=%d(%lu), co=%p", p->index, (uint64_t)p->thread_id, p->coroutine);
    //            // }
    //
    //            assert(p->status == P_STATUS_RUNNING);
    //
    //            // 开始抢占
    //            DEBUGF(
    //                    "[wait_sysmon.share.thread_locker] p_index=%d(%lu), p_status=%d co=%p run timeout(%lu ms), will send SIGURG",
    //                    p->index,
    //                    (uint64_t) p->thread_id, p->status, p->coroutine, time / 1000 / 1000);
    //
    //            // 发送信号强制中断线程
    //            if (pthread_kill(p->thread_id, SIGURG) != 0) {
    //                assert(false && "error sending SIGURG to thread");
    //            }
    //
    //            // 直接更新 can_preempt 的值以及清空 co_start_t 避免被重复抢占
    //            DEBUGF("[wait_sysmon.share.thread_locker] p_index=%d send SIGURG success", p->index);
    //            p->status = P_STATUS_PREEMPT;
    //        SHARE_UNLOCK_NEXT:
    //            mutex_unlock(&p->thread_locker);
    //            DEBUGF("[wait_sysmon.share.thread_locker] unlocker, p_index_%d=%d", p->share, p->index);
    //        }
}

static void wait_sysmon() {
    // 每 50 * 10ms 进行 eval 一次
    int gc_eval_count = WAIT_MID_TIME;

    // 循环监控(每 10ms 监控一次)
    while (true) {
        processor_sysmon();

        // - GC 判断 (每 100ms 进行一次)
        if (gc_eval_count <= 0) {
            runtime_eval_gc();
            gc_eval_count = WAIT_SHORT_TIME; // 10 * 10ms = 100ms
        }

        gc_eval_count--;

        usleep(WAIT_SHORT_TIME * 1000); // 10ms
    }
}

void wait_sysmond() {
    uv_thread_t thread_id; // 当前 processor 绑定的 pthread 线程

    if (uv_thread_create(&thread_id, wait_sysmon, NULL) != 0) {
        assert(false && "pthread_create failed %s");
    }
    DEBUGF("[wait_sysmond] sysmon thread %ld create success", thread_id)
}