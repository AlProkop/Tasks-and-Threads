/******************************************** 
 * Task version                             *
 *                                          *
 * Author: Aliaksei Prakapenka   1701212    *
 *                                          *
 *******************************************/
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading;
using System.Threading.Tasks;

namespace thread_version
{
    class Program
    {
        static int storage_length;
        static int thread_count;
        static int[] storage;
        static Mutex mutex = new Mutex();

        static void addThreadId(Object obj)
        {
            Random r = new Random();
            int to = r.Next(1, 3);
            Thread.Sleep(to);

            int id = (int)obj;
            for(int i=0; i<storage_length; i++)
            {
                mutex.WaitOne();
                if (storage[i] == 0)
                {
                    Thread.Sleep(to+1);
                    storage[i] = id;
                }
                mutex.ReleaseMutex();
            }
        }

        static void Main(string[] args)
        {
            int i = 0;
            // check first the args count
            if(args.Length != 2)
            {
                Console.WriteLine("Usage: <prog> <product_count> <thread_count>");
                Console.ReadLine();
                return;
            }
            // convert args to int.. use default values if illegal..
            if (!int.TryParse(args[0], out storage_length))
                storage_length = 100;
            if (!int.TryParse(args[1], out thread_count))
                thread_count = 10;
            // now we have legal values, let's create the array..
            storage = new int[storage_length];
            
            // let's create the threads..
            Thread[] thread = new Thread[thread_count];
            for (int j = 0; j < thread_count; j++)
            {
                thread[j] = new Thread(addThreadId);
            }
                // and run the threads..
            for (int j = 0; j < thread_count; j++)
            {
                thread[j].Start(j+1);
            }

            // and wait for their finish..
            for (int j = 0; j < thread_count; j++)
            {
                thread[i].Join();
            }
            // and finally calculate statistics, we will not use the index 0
            int[] products_count = new int[thread_count + 1];

            
            // for loop where all the products counted by producer id
            for (int j = 0; j < storage.Length; j++)
            {
                products_count[storage[j]]++;
            }
            foreach (int item in storage)
            {
                Console.Write(item + " ");
            }
            Console.WriteLine();

            for (int ii=1; ii < products_count.Length; ii++)
            {
                Console.Write(products_count[ii] + " ");
            }
            Console.WriteLine();
            Console.ReadLine();
        }
    }
}
