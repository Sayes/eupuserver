#ifndef MTFD_INCLUDE_AOCSINGLETON_H_
#define MTFD_INCLUDE_AOCSINGLETON_H_

template <class T>
class IAocSingleton {
   private:
    IAocSingleton(const IAocSingleton&);
    IAocSingleton& operator=(const IAocSingleton&);

   public:
    static T* get_instance() {
        if (!instance_) instance_ = new T;
        return instance_;
    }

    static void release() {
        if (instance_) {
            delete instance_;
            instance_ = 0;
        }
    }

   protected:
    IAocSingleton(){};
    ~IAocSingleton(){};
    static T* instance_;
};

template <class T>
T* IAocSingleton<T>::instance_ = 0;

#endif
