#include "../Mutex.h"
#include "boost/enable_shared_from_this.hpp"
#include "boost/bind.hpp"
#include "boost/shared_ptr.hpp"
#include "boost/weak_ptr.hpp"
#include <string>
#include <memory>
#include <map>
using string = std::string;
class Stock{
public:
    Stock(std::string key):key_(key){}
    std::string key(){
        return key_;
    }
private:
    float price_;
    std::string key_;
};

class StockFactory:public boost::enable_shared_from_this<StockFactory>,boost::noncopyable{
public:
    boost::shared_ptr<Stock> get(const std::string &key){
        boost::shared_ptr<Stock> pStock;

        MutexLockGuard lock(mutex_);

        //提升
        boost::weak_ptr<Stock>& wkStock = stocks_[key]; //wkStock是引用

        pStock = wkStock.lock();//尝试提升

        //提升失败
        if(!pStock){
            pStock.reset(new Stock(key),boost::bind(&StockFactory::weakDeleteCallback,
                                    boost::weak_ptr<StockFactory>(shared_from_this()),_1));

            //this强制转换为weak_ptr
            //wkStock是引用 实际上等价于stocks_[key]=pStock
            wkStock = pStock;
        }

        return pStock;



    }
private:
    void weakDeleteCallback(const std::weak_ptr<StockFactory>& wkFactory,Stock* stock){
        std::shared_ptr<StockFactory> factory(wkFactory.lock());//尝试提升，看看factory对象是否存在

        if(factory){
            factory->removeStock(stock);
        }
        delete stock;

    }

    void removeStock(Stock *stock){
        if(stock){
            MutexLockGuard lock(mutex_);
            stocks_.erase(stock->key());
        }

    }

private:
    mutable MutexLock mutex_; //可变的
    std::map<std::string,boost::weak_ptr<Stock>> stocks_;
}; 

int main(){
    
}

