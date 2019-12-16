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
        static int task_count;
        static int[] storage;

        static void Main(string[] args)
        {  //encapsulate a method which gets id as a parameter 
            Action<object> addThreadId = (object obj) =>
            {
                int id = (int)obj;
                for (int i = 0; i < storage_length; i++)
                {   // acquire a lock on an array elements is the loop
                    Monitor.Enter(storage);
                    if (storage[i] == 0)
                    {
                        Thread.Sleep(100);
                        storage[i] = id;
                    }
                    Monitor.Exit(storage);
                }
            };


            // check first the args count
            if (args.Length != 2)
            {
                Console.WriteLine("Usage: <prog> <product_count> <thread_count>");
                Console.ReadLine();
                return;
            }
            // convert args to int.. use default values if illegal..
            if (!int.TryParse(args[0], out storage_length))
                storage_length = 100;
            if (!int.TryParse(args[1], out task_count))
                task_count = 10;
            // now we have legal values, let's create the array..
            storage = new int[storage_length];
            // let's create the tasks..
          
            Task[] tasks = new Task[task_count];
            for (int i = 0; i < task_count; i++)
            {
                tasks[i] = new Task(addThreadId, i + 1); //create a task which uses id parameter
            }

            // and run the tasks..
            foreach (var t in tasks)
                t.Start();
            // and wait for their finish..
            Task.WaitAll(tasks);

            // and finally calculate statistics, we will not use the index 0
            int[] products_count = new int[task_count + 1];
            

            // for loop where all the products counted by producer id
            for (int i = 0; i < storage.Length; i++)
            {
                products_count[storage[i]]++;
            }
            foreach (int item in storage)
            {
                Console.Write(item + " ");
            }
            Console.WriteLine();
            for (int ii = 1; ii < products_count.Length; ii++)
            {
                Console.Write(products_count[ii] + " ");
            }
            Console.WriteLine();
            Console.ReadLine();
        }
    }
}
