/**
 * File: thread-pool.cc
 * --------------------
 * Presents the implementation of the ThreadPool class.
 */

#include "thread-pool.h"
using namespace std;

// Constructor: inicia despachador y hilos trabajadores
ThreadPool::ThreadPool(size_t numThreads) : wts(numThreads), done(false) {
    // Inicializar cada trabajador y su hilo
    for (size_t i = 0; i < wts.size(); ++i) {
        wts[i].available = true;
        wts[i].ts = thread(&ThreadPool::worker, this, (int)i);
    }

    // Iniciar el hilo despachador
    dt = thread(&ThreadPool::dispatcher, this);
}

// Programa una tarea en la cola y notifica al despachador
void ThreadPool::schedule(const function<void()>& thunk) {
    if (!thunk) throw invalid_argument("Tarea vacía no permitida");
    // Protege acceso a la cola de tareas y flag done
    {
        lock_guard<mutex> lk(queueLock_);
        if (done) throw runtime_error("No se pueden programar tareas: pool detenido");
        taskQueue.push_back(thunk);
    }
    // Incrementa contador de tareas en vuelo
    {
        lock_guard<mutex> lk(waitLock_);
        ++tasksInFlight_;
    }
    queueCv_.notify_one(); // señalizar al despachador
}

// Hilo despachador: espera tareas y las asigna a trabajadores disponibles
void ThreadPool::dispatcher() {
   while (true) {
        function<void(void)> task;
        {
            unique_lock<mutex> lk(queueLock_);
            queueCv_.wait(lk, [this]{ return done || !taskQueue.empty(); });
            if (done && taskQueue.empty()) break;
            task = move(taskQueue.front());
            taskQueue.pop_front();
        }
        // Buscar un trabajador disponible
        bool assigned = false;
        while (!assigned) {
            if (done) break;
            for (size_t i = 0; i < wts.size(); ++i) {
                lock_guard<mutex> lk(wts[i].mtx);
                if (wts[i].available) {
                    wts[i].available = false;
                    wts[i].thunk = move(task);
                    wts[i].sem.signal();
                    assigned = true;
                    break;
                }
            }
            if (!assigned) this_thread::yield();
        }
    }
}

// Función que ejecuta cada trabajador
void ThreadPool::worker(int id) {
  while (true) {
        wts[id].sem.wait(); // esperar señal de nueva tarea o cierre
        function<void(void)> fn;
        {
            lock_guard<mutex> lk(wts[id].mtx);
            if (done && !wts[id].thunk) break;
            fn = move(wts[id].thunk);
        }
        if (fn) fn();
        {
            lock_guard<mutex> lk(waitLock_);
            if (--tasksInFlight_ == 0) waitCv_.notify_all();
        }
        {
            lock_guard<mutex> lk(wts[id].mtx);
            wts[id].available = true;
        }
    }
}

// Bloquea hasta que todas las tareas en vuelo finalicen
void ThreadPool::wait() {
    unique_lock<mutex> lk(waitLock_);
    waitCv_.wait(lk, [this]{ return tasksInFlight_ == 0; });
}

// Destructor: espera la finalización de tareas y cierra el pool
ThreadPool::~ThreadPool() {
    // 1) Esperar a que todas las tareas en vuelo terminen
    wait();
    // 2) Indicar cierre al despachador
    {
        lock_guard<mutex> lk(queueLock_);
        done = true;
    }
    queueCv_.notify_one();
    // 3) Esperar a que el hilo despachador termine
    if (dt.joinable()) dt.join();
    // 4) Señalizar a todos los trabajadores para que salgan
    for (auto &w : wts) w.sem.signal();
    // 5) Unir hilos trabajadores
    for (auto &w : wts) if (w.ts.joinable()) w.ts.join();
}