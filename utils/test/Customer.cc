#include <map>
#include <string>
#include <vector>

#include <boost/shared_ptr.hpp>

#include "../Mutex.h"

using std::string;

class CustomerData : boost::noncopyable
{
 public:
  CustomerData()
    : data_(new Map)
  { }

  int query(const string& customer, const string& stock) const;

 private:
  typedef std::pair<string, int> Entry;
  typedef std::vector<Entry> EntryList;
  typedef std::map<string, EntryList> Map;
  typedef boost::shared_ptr<Map> MapPtr;
  void update(const string& customer, const EntryList& entries);
  void update(const string& message);

  static int findEntry(const EntryList& entries, const string& stock){
    //得到一个vector<pair<string,int>>
    for(auto entry : entries){
      if(entry.first==stock){
        return entry.second;
      }
    }
    return 0;
  }
  static MapPtr parseData(const string& message);

  MapPtr getData() const
  {
    MutexLockGuard lock(mutex_);
    return data_;
  }

  mutable MutexLock mutex_;
  MapPtr data_;
};

int CustomerData::query(const string& customer, const string& stock) const
{
  MapPtr data = getData();

  Map::const_iterator entries = data->find(customer);
  if (entries != data->end())
    return findEntry(entries->second, stock);
  else
    return -1;
}

void CustomerData::update(const string& customer, const EntryList& entries)
{
  MutexLockGuard lock(mutex_);
  if (!data_.unique())
  {
    //为什么不用reset？ 读端并没有拷贝一份 因此没有引用加1一说
    // copy on other readning
    MapPtr newData(new Map(*data_));
    data_.swap(newData);
  }
  assert(data_.unique());
  (*data_)[customer] = entries;
  printf("updated\n");
}

// void CustomerData::update(const string& message)
// {
//   MapPtr newData = parseData(message);
//   if (newData)
//   {
//     MutexLockGuard lock(mutex_);
//     data_.swap(newData);
//   }
// }

int main()
{
  CustomerData data;
}
